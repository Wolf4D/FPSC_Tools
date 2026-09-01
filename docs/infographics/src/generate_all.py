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
BG = (255, 255, 255)
CARD_BG = (248, 250, 252)
TEXT_MAIN = (15, 23, 42)
TEXT_MUTED = (71, 85, 105)

COL_BLUE = (2, 132, 199)
COL_GREEN = (22, 163, 74)
COL_ORANGE = (234, 88, 12)
COL_PURPLE = (147, 51, 234)
COL_RED = (225, 29, 72)
COL_AMBER = (217, 119, 6)
COL_CYAN = (8, 145, 178)
COL_GRAY = (100, 116, 139)

FT = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 22)
FB = ImageFont.truetype(r'C:\Windows\Fonts\segoeui.ttf', 16)
FN = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 15)

FT_MODE = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 26)
FB_MODE = ImageFont.truetype(r'C:\Windows\Fonts\segoeui.ttf', 18)
FP_MODE = ImageFont.truetype(r'C:\Windows\Fonts\consola.ttf', 15)

def ptr(d, t, e, c, w=3):
    d.ellipse([t[0]-5, t[1]-5, t[0]+5, t[1]+5], fill=c)
    d.line([t, e], fill=c, width=w)

def card(d, b, n, title, lines, c):
    d.rounded_rectangle(b, 8, fill=CARD_BG, outline=c, width=2)
    x, y = b[0] + 12, b[1] + 10
    if n:
        d.rounded_rectangle([x, y, x + 24, y + 24], 4, fill=c)
        d.text((x + 7, y + 1), n, font=FN, fill=(255, 255, 255))
        d.text((x + 32, y - 2), title, font=FT, fill=TEXT_MAIN)
    else:
        d.text((x, y), title, font=FT, fill=c)
    ly = y + 28
    for l in lines:
        d.text((x, ly), l, font=FB, fill=TEXT_MUTED)
        ly += 22

def hcallout_fanned(d, tip, cy, rx, rw, ch, col, title, params, desc):
    menu_pt = tip
    card_pt = (rx, cy)
    d.line([menu_pt, (menu_pt[0]+20, menu_pt[1]), (rx-20, cy), card_pt], fill=col, width=3)
    d.ellipse([menu_pt[0]-5, menu_pt[1]-5, menu_pt[0]+5, menu_pt[1]+5], fill=col)
    
    box = (rx, cy - ch//2, rx + rw, cy + ch//2)
    d.rounded_rectangle(box, 8, fill=CARD_BG, outline=col, width=2)
    bx, by = box[0] + 14, box[1] + 10
    d.text((bx, by), title, font=FT_MODE, fill=col)
    tw = FT_MODE.getlength(title)
    if params:
        d.text((bx + tw + 15, by + 6), params, font=FP_MODE, fill=TEXT_MUTED)
    d.text((bx, by + 34), desc, font=FB_MODE, fill=TEXT_MUTED)

def generate_toolbar_overview():
    sc = 1.4
    tb = Image.open(os.path.join(ASSETS_DIR, 'toolbar_full.png'))
    tw, th = int(tb.width * sc), int(tb.height * sc)
    tb = tb.resize((tw, th), Image.LANCZOS)

    W, H = 1000, 390
    img = Image.new('RGB', (W, H), BG)
    d = ImageDraw.Draw(img)
    tx = (W - tw) // 2
    ty = 135
    img.paste(tb, (tx, ty))

    pf = (tx + int(263 * sc), ty + int(11 * sc))
    pp = (tx + int(240 * sc), ty + int(11 * sc))
    pl = (tx + int(48 * sc), ty + int(66 * sc))
    pn = (tx + int(115 * sc), ty + int(38 * sc))
    pc = (tx + int(115 * sc), ty + int(67 * sc))
    pr = (tx + int(235 * sc), ty + int(38 * sc))
    ps = (tx + int(235 * sc), ty + int(67 * sc))

    # Top Left
    b1 = (20, 20, 380, 95)
    card(d, b1, '1', 'Fold to Mini-Widget', ['Collapse and restore in one click.'], COL_CYAN)
    mi = Image.open(os.path.join(ASSETS_DIR, 'mini_icon.png')).resize((40, 40), Image.LANCZOS)
    img.paste(mi, (330, 28), mi.convert('RGBA'))
    ptr(d, pf, (380, 57), COL_CYAN)

    # Top Right
    b2 = (650, 20, 980, 95)
    card(d, b2, '2', 'Window Controls', ['Pin | Settings | Close'], COL_AMBER)
    ptr(d, pp, (650, 57), COL_AMBER)

    # Far Left
    b3 = (20, 150, 220, 245)
    card(d, b3, '3', 'Status LED', ['Red = Stopped', 'Green = Running'], COL_GREEN)
    ptr(d, pl, (220, pl[1]), COL_GREEN)

    # Bottom row
    by1, by2 = 295, 370
    cw = 220; gap = 20
    b4 = (20, by1, 20+cw, by2)
    card(d, b4, '4', 'Lightmap', ['5 bake presets'], COL_BLUE)
    ptr(d, pn, (20+cw//2, by1), COL_BLUE)

    b5 = (20+cw+gap, by1, 20+2*cw+gap, by2)
    card(d, b5, '5', 'Cleaner', ['Purge build cache'], COL_ORANGE)
    ptr(d, pc, (20+cw+gap+cw//2, by1), COL_ORANGE)

    b6 = (20+2*(cw+gap), by1, 20+3*cw+2*gap, by2)
    card(d, b6, '6', 'Level Stash', ['Snapshot + .fpm'], COL_PURPLE)
    ptr(d, ps, (20+2*(cw+gap)+cw//2, by1), COL_PURPLE)

    b7 = (20+3*(cw+gap), by1, 20+4*cw+3*gap, by2)
    card(d, b7, '7', 'Restart', ['Kill and relaunch'], COL_RED)
    ptr(d, pr, (20+3*(cw+gap)+cw//2, by1), COL_RED)

    out_path = os.path.join(OUTPUT_DIR, 'toolbar_overview.png')
    img.save(out_path, quality=95)
    print('[+] Generated:', out_path)

def make_mode_img(filename, menu_img_name, items):
    sc_m = 1.6
    m = Image.open(os.path.join(ASSETS_DIR, menu_img_name))
    mw, mh = int(m.width * sc_m), int(m.height * sc_m)
    m = m.resize((mw, mh), Image.LANCZOS)
    
    W, H = 1200, max(500, len(items)*100 + 40)
    img_m = Image.new('RGB', (W, H), BG)
    dm = ImageDraw.Draw(img_m)
    
    mx = 30; my = (H - mh) // 2
    img_m.paste(m, (mx, my))
    tip_x = mx + int(m.width * 0.9 * sc_m) 
    if 'clean' in menu_img_name: tip_x = mx + int(352 * sc_m)
    if 'profiles' in menu_img_name: tip_x = mx + int(331 * sc_m)
    if 'stash' in menu_img_name: tip_x = mx + int(410 * sc_m)
    
    rx = tip_x + 60
    rw = W - rx - 30
    ch = 76
    
    start_y = (H - (len(items)*90 - 10)) // 2 + 10
    
    for i, (orig_y, col, t, par, desc) in enumerate(items):
        menu_y = my + int(orig_y * sc_m)
        card_cy = start_y + i * 90
        hcallout_fanned(dm, (tip_x, menu_y), card_cy, rx, rw, ch, col, t, par, desc)
        
    out_path = os.path.join(OUTPUT_DIR, filename)
    img_m.save(out_path, quality=95)
    print('[+] Generated:', out_path)


def generate_mode_lightmapping():
    items2 = [
        (92,  COL_GRAY,  'Disabled', 'lightmapping = 0', 'No lightmaps. Dynamic lighting only.'),
        (120, COL_GREEN, 'Fast', 'tex: 128 | qual: 5 | ao: 0', 'Draft bake for quick gameplay tests.'),
        (148, COL_BLUE,  'Normal (Def)', 'tex: 256 | qual: 16 | ao: 0', 'Balanced preset for everyday level design.'),
        (176, COL_PURPLE, 'Release', 'tex: 256 | qual: 50 | ao: 1', 'High visual fidelity for final game builds.'),
        (204, COL_AMBER, 'Ultra', 'tex: 512 | qual: 100 | ao: 1', 'Max quality + full Ambient Occlusion.'),
    ]
    make_mode_img('mode_lightmapping.png', 'menu_profiles.png', items2)

def generate_mode_cleaner():
    items3 = [
        (113.5, COL_ORANGE, '.bin / .dbo', 'Files/**/*.bin, .dbo', 'Deletes compiled meshes & segment binaries.'),
        (141.5, COL_AMBER,  'Level Build', 'levelbank/testlevel/', 'Purges intermediate build & lightmap files.'),
        (168,   COL_RED,    'All (Deep)', 'Full Cache Purge', 'Complete cleanup of models, build data, lightmaps.'),
        (205,   COL_BLUE,   'Temp Cache', '%TEMP%/dbpdata*', 'Frees DarkBASIC temporary runtime folders.'),
    ]
    make_mode_img('mode_cleaner.png', 'menu_clean.png', items3)

def generate_mode_stash():
    items4 = [
        (120, COL_PURPLE, 'Quick Save', '1-Click Snapshot', 'Backs up testlevel/ + auto-copies active .fpm map.'),
        (148, COL_BLUE,   'Restore', 'Instant Rollback', 'Reverts to latest snapshot + restores .fpm map.'),
        (185, COL_AMBER,  'Manager...', 'GUI Dialog', 'Browse snapshots with dates, sizes, rename & restore.'),
        (213, COL_GRAY,   'Stashes Dir', 'Explorer Folder', 'Opens physical stashes/ folder in Windows Explorer.'),
    ]
    make_mode_img('mode_stash.png', 'menu_stash.png', items4)


if __name__ == '__main__':
    print('Generating all FPSC Tools infographics...')
    generate_toolbar_overview()
    generate_mode_lightmapping()
    generate_mode_cleaner()
    generate_mode_stash()
    print('Done!')
