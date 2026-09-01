# =============================================================================
# FPSC Tools - Infographics Generator
# Madness Studio • Ivan Klenov (aka Navy LiK)
# =============================================================================

import os
from PIL import Image, ImageDraw, ImageFont

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
ASSETS_DIR = os.path.join(BASE_DIR, 'assets')
OUTPUT_DIR = os.path.abspath(os.path.join(BASE_DIR, '..'))

# Colors (Pure White Theme)
BG_WHITE = (255, 255, 255)
CARD_BG = (248, 250, 252)        # slate-50
TEXT_MAIN = (15, 23, 42)         # slate-900
TEXT_MUTED = (71, 85, 105)       # slate-600

COL_BLUE = (2, 132, 199)         # sky-600
COL_GREEN = (22, 163, 74)        # green-600
COL_ORANGE = (234, 88, 12)       # orange-600
COL_PURPLE = (147, 51, 234)      # purple-600
COL_RED = (225, 29, 72)          # rose-600
COL_AMBER = (217, 119, 6)        # amber-600
COL_CYAN = (8, 145, 178)         # cyan-600
COL_GRAY = (100, 116, 139)       # slate-500

font_title = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 14)
font_body = ImageFont.truetype(r'C:\Windows\Fonts\segoeui.ttf', 12)
font_num = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 11)
font_params = ImageFont.truetype(r'C:\Windows\Fonts\consola.ttf', 11)

def draw_straight_pointer(draw, pt_tip, pt_card, color, width=2):
    draw.ellipse((pt_tip[0]-4, pt_tip[1]-4, pt_tip[0]+4, pt_tip[1]+4), fill=color)
    draw.line([pt_tip, pt_card], fill=color, width=width)

def draw_card(draw, box, num_str, title, lines, color):
    draw.rounded_rectangle(box, radius=6, fill=CARD_BG, outline=color, width=1)
    bx, by = box[0] + 8, box[1] + 6
    if num_str:
        draw.rounded_rectangle((bx, by, bx + 16, by + 16), radius=3, fill=color)
        draw.text((bx + 4, by + 1), num_str, font=font_num, fill=(255, 255, 255))
        draw.text((bx + 22, by + 1), title, font=font_title, fill=TEXT_MAIN)
    else:
        draw.text((bx, by), title, font=font_title, fill=color)
    ly = by + 20
    for line in lines:
        draw.text((bx, ly), line, font=font_body, fill=TEXT_MUTED)
        ly += 15

def draw_horizontal_callout(draw, pt_tip, box, border_col, title, params, desc):
    draw.ellipse((pt_tip[0]-4, pt_tip[1]-4, pt_tip[0]+4, pt_tip[1]+4), fill=border_col)
    draw.line([pt_tip, (box[0], pt_tip[1])], fill=border_col, width=2)
    draw.rounded_rectangle(box, radius=6, fill=CARD_BG, outline=border_col, width=1)
    bx, by = box[0] + 10, box[1] + 7
    draw.text((bx, by), title, font=font_title, fill=border_col)
    t_w = font_title.getlength(title)
    if params:
        draw.text((bx + max(t_w + 14, 185), by + 1), params, font=font_params, fill=TEXT_MUTED)
    draw.text((bx, by + 19), desc, font=font_body, fill=TEXT_MUTED)


def generate_toolbar_overview():
    scale = 2.0
    tb_raw = Image.open(os.path.join(ASSETS_DIR, 'toolbar_full.png'))
    tb_w, tb_h = int(tb_raw.width * scale), int(tb_raw.height * scale)
    tb = tb_raw.resize((tb_w, tb_h), Image.LANCZOS)

    canvas_w, canvas_h = 1260, 560
    img = Image.new('RGB', (canvas_w, canvas_h), BG_WHITE)
    draw = ImageDraw.Draw(img)

    tb_x = (canvas_w - tb_w) // 2   # 314
    tb_y = 180
    img.paste(tb, (tb_x, tb_y))

    pt_fold = (tb_x + int(263 * scale), tb_y + int(11 * scale))
    pt_pin = (tb_x + int(240 * scale), tb_y + int(11 * scale))
    pt_led = (tb_x + int(48 * scale), tb_y + int(66 * scale))

    pt_normal = (tb_x + int(115 * scale), tb_y + int(38 * scale))
    pt_clean = (tb_x + int(115 * scale), tb_y + int(67 * scale))
    pt_restart = (tb_x + int(235 * scale), tb_y + int(38 * scale))
    pt_stash = (tb_x + int(235 * scale), tb_y + int(67 * scale))

    # 1. Fold to Mini-Widget
    b1 = (40, 25, 520, 115)
    draw.rounded_rectangle(b1, radius=6, fill=CARD_BG, outline=COL_CYAN, width=1)
    draw.rounded_rectangle((b1[0] + 10, b1[1] + 8, b1[0] + 26, b1[1] + 24), radius=3, fill=COL_CYAN)
    draw.text((b1[0] + 14, b1[1] + 9), '1', font=font_num, fill=(255, 255, 255))
    draw.text((b1[0] + 32, b1[1] + 8), 'Fold to Mini-Widget (Collapse)', font=font_title, fill=TEXT_MAIN)
    draw.text((b1[0] + 10, b1[1] + 30), 'Clicking Fold collapses toolbar into a floating mini-icon', font=font_body, fill=TEXT_MUTED)
    draw.text((b1[0] + 10, b1[1] + 46), 'centered at cursor. Click icon to restore with zero drift.', font=font_body, fill=TEXT_MUTED)

    mini_img = Image.open(os.path.join(ASSETS_DIR, 'mini_icon.png')).resize((55, 55), Image.LANCZOS)
    img.paste(mini_img, (b1[0] + 445, b1[1] + 18), mini_img.convert('RGBA'))
    draw_straight_pointer(draw, pt_fold, (520, 95), COL_CYAN)

    # 2. Window Controls
    b2 = (740, 25, 1220, 115)
    draw_card(draw, b2, '2', 'Window Controls', 
              ['Pin: Toggle Always on Top above all editor windows', 
               'Gear: Open Settings dialog • X: Exit application'], COL_AMBER)
    draw_straight_pointer(draw, pt_pin, (794, 115), COL_AMBER)

    # 3. Status LED
    b3 = (30, 230, 270, 315)
    draw_card(draw, b3, '3', 'Status LED & Launch',
              ['Red: Engine Stopped.', 'Green: Engine Active.', 'Click icon to launch / restart.'], COL_GREEN)
    draw_straight_pointer(draw, pt_led, (270, pt_led[1]), COL_GREEN)

    # Bottom row
    bot_y1, bot_y2 = 440, 525
    b4 = (30, bot_y1, 315, bot_y2)
    draw_card(draw, b4, '4', 'Lightmapping Profiles',
              ['Dropdown with 5 bake presets.', 'Updates setup.ini & reloads engine.'], COL_BLUE)
    draw_straight_pointer(draw, pt_normal, (200, bot_y1), COL_BLUE)

    b5 = (335, bot_y1, 620, bot_y2)
    draw_card(draw, b5, '5', 'Fast Cache Cleaner',
              ['Purges .bin/.dbo models, level build', 'cache data, and engine temp files.'], COL_ORANGE)
    draw_straight_pointer(draw, pt_clean, (475, bot_y1), COL_ORANGE)

    b6 = (640, bot_y1, 925, bot_y2)
    draw_card(draw, b6, '6', 'Level Stash System',
              ['Instant snapshot backup & rollback of', 'testlevel builds + active .fpm map.'], COL_PURPLE)
    draw_straight_pointer(draw, pt_stash, (784, bot_y1), COL_PURPLE)

    b7 = (945, bot_y1, 1230, bot_y2)
    draw_card(draw, b7, '7', 'Instant Restart',
              ['Force-kills hanging FPSC processes', 'and cleanly relaunches the editor.'], COL_RED)
    draw_straight_pointer(draw, pt_restart, (1085, bot_y1), COL_RED)

    out_path = os.path.join(OUTPUT_DIR, 'toolbar_overview.png')
    img.save(out_path, quality=95)
    print('[+] Generated:', out_path)


def generate_mode_lightmapping():
    scale = 2.0
    card_h = 50
    m_raw2 = Image.open(os.path.join(ASSETS_DIR, 'menu_profiles.png'))
    m_w2, m_h2 = int(m_raw2.width * scale), int(m_raw2.height * scale)
    m2 = m_raw2.resize((m_w2, m_h2), Image.LANCZOS)

    cw2, ch2 = 1250, 560
    img2 = Image.new('RGB', (cw2, ch2), BG_WHITE)
    draw2 = ImageDraw.Draw(img2)
    mx2, my2 = 40, (ch2 - m_h2) // 2
    img2.paste(m2, (mx2, my2))

    tip_x2 = mx2 + int(331 * scale)
    rx2 = tip_x2 + 30
    rw2 = 450

    items2 = [
        (my2 + int(92.0 * scale), COL_GRAY, 'Disabled Preset', 'lightmapping = 0', 'Lightmapping disabled completely. Uses real-time dynamic vertex lighting.'),
        (my2 + int(120.0 * scale), COL_GREEN, 'Fast Preset', 'tex: 128 | qual: 5 | ao: 0', 'Low-resolution draft bake for rapid playthroughs with minimal wait time.'),
        (my2 + int(148.0 * scale), COL_BLUE, 'Normal Preset (Default)', 'tex: 256 | qual: 16 | ao: 0', 'Balanced bake preset for everyday level geometry and lighting placement.'),
        (my2 + int(176.0 * scale), COL_PURPLE, 'Release Preset', 'tex: 256 | qual: 50 | ao: 1', 'High visual fidelity with smooth shadows. Recommended for playable releases.'),
        (my2 + int(204.0 * scale), COL_AMBER, 'Ultra Preset', 'tex: 512 | qual: 100 | ao: 1', 'Maximum shadow quality and full Ambient Occlusion. Ideal for final showcase.')
    ]

    for tip_y, col, title, params, desc in items2:
        box = (rx2, tip_y - card_h//2, rx2 + rw2, tip_y + card_h//2)
        draw_horizontal_callout(draw2, (tip_x2, tip_y), box, col, title, params, desc)

    out_path = os.path.join(OUTPUT_DIR, 'mode_lightmapping.png')
    img2.save(out_path, quality=95)
    print('[+] Generated:', out_path)


def generate_mode_cleaner():
    scale = 2.0
    card_h = 52
    m_raw3 = Image.open(os.path.join(ASSETS_DIR, 'menu_clean.png'))
    m_w3, m_h3 = int(m_raw3.width * scale), int(m_raw3.height * scale)
    m3 = m_raw3.resize((m_w3, m_h3), Image.LANCZOS)

    cw3, ch3 = 1500, 560
    img3 = Image.new('RGB', (cw3, ch3), BG_WHITE)
    draw3 = ImageDraw.Draw(img3)
    mx3, my3 = 40, (ch3 - m_h3) // 2
    img3.paste(m3, (mx3, my3))

    tip_x3 = mx3 + int(352 * scale)
    rx3 = tip_x3 + 35
    rw3 = 630

    items3 = [
        (my3 + int(113.5 * scale), COL_ORANGE, '.bin and .dbo Cleaner', 'Target: Files/**/*.bin, Files/**/*.dbo', 'Deletes compiled model meshes (.dbo) & segment binaries (.bin) across Files/. Forces fresh rebuild.'),
        (my3 + int(141.5 * scale), COL_AMBER, 'Level Build Data', 'Target: levelbank/testlevel/ (*.dbo, .ele, .lgt, .dat)', 'Purges intermediate level builder files & lightmap texture caches from testlevel directory.'),
        (my3 + int(168.0 * scale), COL_RED, 'All (Deep Clean)', 'Target: Combined .bin/.dbo + Build Data + Lightmaps', 'Comprehensive one-click purge. Resolves phantom crashes, visual anomalies, and editor desyncs.'),
        (my3 + int(205.0 * scale), COL_BLUE, 'Engine Temp Cache', 'Target: %TEMP%/dbpdata*', 'Safely deletes gigabytes of leftover DarkBASIC Professional temporary runtime directories.')
    ]

    for tip_y, col, title, params, desc in items3:
        box = (rx3, tip_y - card_h//2, rx3 + rw3, tip_y + card_h//2)
        draw_horizontal_callout(draw3, (tip_x3, tip_y), box, col, title, params, desc)

    out_path = os.path.join(OUTPUT_DIR, 'mode_cleaner.png')
    img3.save(out_path, quality=95)
    print('[+] Generated:', out_path)


def generate_mode_stash():
    scale = 2.0
    card_h = 52
    m_raw4 = Image.open(os.path.join(ASSETS_DIR, 'menu_stash.png'))
    m_w4, m_h4 = int(m_raw4.width * scale), int(m_raw4.height * scale)
    m4 = m_raw4.resize((m_w4, m_h4), Image.LANCZOS)

    cw4, ch4 = 1600, 600
    img4 = Image.new('RGB', (cw4, ch4), BG_WHITE)
    draw4 = ImageDraw.Draw(img4)
    mx4, my4 = 40, (ch4 - m_h4) // 2
    img4.paste(m4, (mx4, my4))

    tip_x4 = mx4 + int(410 * scale)
    rx4 = tip_x4 + 35
    rw4 = 650

    items4 = [
        (my4 + int(120.0 * scale), COL_PURPLE, 'Quick Save (Quick Stash)', 'One-click level snapshot', 'Archives testlevel/ into stashes/YYYY-MM-DD_HH-MM-SS/ AND automatically captures open .fpm map!'),
        (my4 + int(148.0 * scale), COL_BLUE, 'Restore Latest Snapshot', 'Rollback to previous build', 'Reverts testlevel/ to the latest snapshot and restores associated source .fpm map file in 1 click.'),
        (my4 + int(185.0 * scale), COL_AMBER, 'Stash Manager...', 'Interactive GUI dialog', 'Opens management dialog with timestamps, disk size, entity counts, custom renaming, and restore targets.'),
        (my4 + int(213.0 * scale), COL_GRAY, 'Open Stashes Folder', 'Folder in Windows Explorer', 'Opens stashes/ directory in Windows Explorer for external backup, archiving, and project sharing.')
    ]

    for tip_y, col, title, params, desc in items4:
        box = (rx4, tip_y - card_h//2, rx4 + rw4, tip_y + card_h//2)
        draw_horizontal_callout(draw4, (tip_x4, tip_y), box, col, title, params, desc)

    out_path = os.path.join(OUTPUT_DIR, 'mode_stash.png')
    img4.save(out_path, quality=95)
    print('[+] Generated:', out_path)


if __name__ == '__main__':
    print('Generating all FPSC Tools infographics...')
    generate_toolbar_overview()
    generate_mode_lightmapping()
    generate_mode_cleaner()
    generate_mode_stash()
    print('Done!')
