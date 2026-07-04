#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
learn_music_site_probe.py — 学习：如何判断音乐网站能否搜歌 + 播放

用法（在项目根目录或 scripts 目录）:
    python scripts/learn_music_site_probe.py
    python scripts/learn_music_site_probe.py --keyword 周杰伦 --page 1

流程对应我们集成 MyFreeMp3 时的验证步骤:
  1. 探测搜索入口是否可访问
  2. 从 HTML 解析歌曲列表（无 JSON API 时）
  3. 请求详情页，提取 APlayer 里的 stream URL
  4. 对比「无 Referer / 有 Referer」能否 200
  5. 输出结论与集成建议

依赖: 仅 Python 3 标准库（无需 pip install）
"""

from __future__ import annotations

import argparse
import re
import ssl
import sys
import urllib.error
import urllib.parse
import urllib.request
from dataclasses import dataclass, field
from typing import List, Optional

BASE_URL = "https://myfreemp3online.com"
USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicSiteProbe/1.0"
REFERER = BASE_URL + "/"

# 与 C++ MyFreeMp3Parser 相同的正则思路（使用 re 而非 raw string 陷阱）
TRACK_LINK_RE = re.compile(
    r'href="https?://myfreemp3online\.com/song/(\d+)\.html"\s+'
    r'class="text-primary font-weight-bold"[^>]*title="([^"]*)"',
    re.IGNORECASE,
)
STREAM_URL_RE = re.compile(r"url:\s*'([^']+)'")
ACTIVE_PAGE_RE = re.compile(r'<li\s+class="active"><span>(\d+)</span></li>')
TAG_RE = re.compile(r"<[^>]+>")


@dataclass
class ProbeResult:
    keyword: str
    page: int
    search_ok: bool = False
    track_count: int = 0
    sample_song_id: str = ""
    sample_title: str = ""
    stream_url: str = ""
    direct_status: Optional[int] = None
    referer_status: Optional[int] = None
    verdict: str = ""
    notes: List[str] = field(default_factory=list)


def http_get(url: str, headers: Optional[dict] = None, method: str = "GET") -> tuple[int, bytes, dict]:
    """发起 HTTP 请求，返回 (status_code, body, response_headers)"""
    req = urllib.request.Request(url, method=method, headers=headers or {})
    ctx = ssl.create_default_context()
    try:
        with urllib.request.urlopen(req, context=ctx, timeout=20) as resp:
            return resp.status, resp.read(), dict(resp.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read(), dict(e.headers)


def strip_html_tags(text: str) -> str:
    return TAG_RE.sub("", text).strip()


def build_search_url(keyword: str, page: int) -> str:
    params = {"q": keyword}
    if page > 1:
        params["page"] = str(page)
    return f"{BASE_URL}/search.php?{urllib.parse.urlencode(params)}"


def parse_search_html(html: str) -> tuple[List[tuple[str, str]], int]:
    """返回 ([(song_id, title), ...], current_page)"""
    tracks: List[tuple[str, str]] = []
    for m in TRACK_LINK_RE.finditer(html):
        song_id = m.group(1)
        raw_title = m.group(2).split(" MP3")[0].strip()
        tracks.append((song_id, strip_html_tags(raw_title)))

    page = 1
    pm = ACTIVE_PAGE_RE.search(html)
    if pm:
        page = int(pm.group(1))
    return tracks, page


def parse_detail_stream_url(html: str) -> str:
    m = STREAM_URL_RE.search(html)
    return m.group(1).strip() if m else ""


def probe_head(url: str, referer: Optional[str] = None) -> int:
    headers = {"User-Agent": USER_AGENT}
    if referer:
        headers["Referer"] = referer
    status, _, _ = http_get(url, headers=headers, method="HEAD")
    # 部分 CDN 不支持 HEAD，回退 GET 只读 header
    if status in (405, 501):
        status, body, _ = http_get(url, headers=headers, method="GET")
        _ = body[:1]  # 触发读取即可，不必下载全文
    return status


def run_probe(keyword: str, page: int) -> ProbeResult:
    result = ProbeResult(keyword=keyword, page=page)
    print("=" * 60)
    print(f"Step 1 · 搜索入口探测  keyword={keyword!r}  page={page}")
    print("=" * 60)

    search_url = build_search_url(keyword, page)
    print(f"  URL: {search_url}")

    status, body, _ = http_get(
        search_url,
        headers={"User-Agent": USER_AGENT, "Referer": REFERER},
    )
    if status != 200:
        result.verdict = "FAIL"
        result.notes.append(f"搜索页 HTTP {status}")
        print(f"  ✗ 搜索失败 HTTP {status}")
        return result

    html = body.decode("utf-8", errors="replace")
    tracks, current_page = parse_search_html(html)
    result.search_ok = True
    result.track_count = len(tracks)
    print(f"  ✓ 搜索成功，本页 {len(tracks)} 条，当前页码 {current_page}")

    if not tracks:
        result.verdict = "PARTIAL"
        result.notes.append("搜索页可访问但未解析到歌曲链接（可能 HTML 结构已变）")
        print("  ⚠ 未解析到歌曲，请检查 TRACK_LINK_RE 正则")
        return result

    # 取第一条作为样本
    song_id, title = tracks[0]
    result.sample_song_id = song_id
    result.sample_title = title
    print(f"  样本: [{song_id}] {title}")

    print()
    print("=" * 60)
    print("Step 2 · 详情页提取播放 URL")
    print("=" * 60)

    detail_url = f"{BASE_URL}/song/{song_id}.html"
    print(f"  URL: {detail_url}")

    status, body, _ = http_get(
        detail_url,
        headers={"User-Agent": USER_AGENT, "Referer": REFERER},
    )
    if status != 200:
        result.verdict = "PARTIAL"
        result.notes.append(f"详情页 HTTP {status}")
        print(f"  ✗ 详情页失败 HTTP {status}")
        return result

    detail_html = body.decode("utf-8", errors="replace")
    stream_url = parse_detail_stream_url(detail_html)
    result.stream_url = stream_url

    if not stream_url:
        result.verdict = "PARTIAL"
        result.notes.append("详情页无 stream URL（APlayer url: 字段可能已改）")
        print("  ✗ 未找到 url: '...' 播放地址")
        return result

    print(f"  ✓ 播放地址: {stream_url[:80]}{'...' if len(stream_url) > 80 else ''}")

    print()
    print("=" * 60)
    print("Step 3 · CDN 访问测试（403 问题的根源）")
    print("=" * 60)

    result.direct_status = probe_head(stream_url)
    result.referer_status = probe_head(stream_url, referer=REFERER)

    print(f"  无 Referer : HTTP {result.direct_status}")
    print(f"  有 Referer : HTTP {result.referer_status}")

    print()
    print("=" * 60)
    print("Step 4 · 结论与集成建议")
    print("=" * 60)

    if result.referer_status == 200 and result.direct_status == 403:
        result.verdict = "OK_WITH_PREFETCH"
        result.notes.append("CDN 需要 Referer；Qt QMediaPlayer 不能直接 setSource(url)")
        result.notes.append("方案: QNetworkAccessManager 带 Referer 下载到缓存 → 本地播放")
        print("  ✓ 可集成：搜索 + 详情解析 + Referer 预取后播放")
        print("  ⚠ 不能直接 QMediaPlayer::setSource(https://...) — 会 403")
    elif result.referer_status == 200 and result.direct_status == 200:
        result.verdict = "OK_DIRECT"
        result.notes.append("CDN 允许直连，可尝试 QMediaPlayer::setSource")
        print("  ✓ 可集成：甚至可直接串流播放")
    else:
        result.verdict = "FAIL_PLAYBACK"
        result.notes.append("即使带 Referer 也无法访问音频")
        print("  ✗ 播放 URL 不可访问，需进一步调查 Cookie/Token")

    print()
    print("Step 5 · 格式提示")
    ext = stream_url.rsplit(".", 1)[-1].lower() if "." in stream_url else "?"
    print(f"  扩展名: .{ext}  → Qt6+FFmpeg 通常可解码 mp3/m4a/aac")

    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="学习：音乐网站可播放性探测")
    parser.add_argument("--keyword", default="林俊杰", help="搜索关键词")
    parser.add_argument("--page", type=int, default=1, help="页码，从 1 开始")
    args = parser.parse_args()

    print()
    print("Music Site Probe — 学习示例")
    print(f"目标站点: {BASE_URL}")
    print()

    try:
        result = run_probe(args.keyword, args.page)
    except urllib.error.URLError as e:
        print(f"\n网络错误: {e}")
        print("请检查网络连接或站点是否可访问")
        return 1
    except Exception as e:
        print(f"\n异常: {e}")
        return 1

    print()
    print("=" * 60)
    print(f"最终判定: {result.verdict}")
    for note in result.notes:
        print(f"  · {note}")
    print("=" * 60)
    print()
    print("下一步学习建议:")
    print("  1. 改 --keyword 测不同歌手")
    print("  2. 打开 MyFreeMp3Parser.cpp 对比 C++ 正则实现")
    print("  3. 阅读 OnlineStreamLoader.cpp 看 Referer 预取如何接入播放器")
    print()

    return 0 if result.verdict.startswith("OK") else 1


if __name__ == "__main__":
    sys.exit(main())
