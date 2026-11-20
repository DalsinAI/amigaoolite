#!/usr/bin/env python3
"""Oolite's Workbench icons in Soft 3D, the AmiCradle icon standard (amicradle docs/ICON_STANDARD.md):

    python3 icons/soft3d_oolite.py --gfopenup GFOPENUP --amicradle AMICRADLE [--sheet OUT.png]

GFOPENUP is a gfopenup checkout (the Soft 3D kit: tools/soft3d_drives_art.py, soft3d_kinds_art.py and
soft3d_motifs.py), AMICRADLE an amicradle checkout (scripts/icons/amiga_icon.py writes the GlowIcons). As
amigadosbox's icons/soft3d_dosbox.py, from which this is made. Each icon gets its meta (below) and two
frames, 64 x 64:

    icons/Oolite.info                    the drawer: the Soft 3D drawer with the rocket mark
    icons/Oolite-Start.info              Oolite-Start, an IconX script: a space station, its docking slot lit;
                                         chosen, a ship is in the slot
    icons/ReadMe.info                    the ReadMe: the Soft 3D project page
    package/installer/Install-Oolite.info   the installer: the parcel, the arrow going in, the station on its side
    icons/art/oolite.png                 the station, 256 x 256: the installer's badge for port_installer.py

The station is our own drawing of a plain geometric solid. MIT, Copyright (c) 2026 AmiCradle Team.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HULL = (150, 162, 178)
INK = (22, 26, 36)
SPACE = (12, 16, 28)
SLOT = (255, 196, 64)
SHIP = (220, 72, 48)
CARD = (205, 160, 104)

# each icon's meta: what make_package and Workbench read
META = {
    "Oolite.info": {"type": 2, "defaultTool": None, "toolTypes": [], "toolWindow": None, "stack": 4096,
                    "x": 2147483648, "y": 2147483648,
                    "drawer": {"left": 80, "top": 60, "width": 300, "height": 140, "flags": 0, "viewModes": 0,
                               "currentX": 0, "currentY": 0}},
    "Oolite-Start.info": {"type": 4, "defaultTool": "C:IconX",
                          "toolTypes": ["WINDOW=CON:0/0/640/150/Oolite/AUTO/CLOSE/WAIT", "STACK=16384"],
                          "toolWindow": None, "stack": 16384, "x": 12, "y": 12, "drawer": None},
    "ReadMe.info": {"type": 4, "defaultTool": "SYS:Utilities/MultiView", "toolTypes": [], "toolWindow": None,
                    "stack": 4096, "x": 100, "y": 12, "drawer": None},
}


def draw_station(d3, f, small, chosen=False, at=(256, 256), r=230):
    """The station: a square face with its docking slot, the triangular faces round it, seen from the front."""
    cx, cy = at
    q = r * 0.5                                                               # half the front square
    f.poly([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], d3.dark(HULL, 0.35))   # the outline
    faces = (((-1, -1), (1, -1), (0, -2), d3.light(HULL, 0.3)),               # the four faces round the square,
             ((1, -1), (1, 1), (2, 0), d3.dark(HULL, 0.12)),                  # lit from the top left
             ((1, 1), (-1, 1), (0, 2), d3.dark(HULL, 0.25)),
             ((-1, 1), (-1, -1), (-2, 0), d3.light(HULL, 0.15)))
    for (ax, ay), (bx, by), (tx, ty), colour in faces:
        f.poly([(cx + ax * q, cy + ay * q), (cx + bx * q, cy + by * q), (cx + tx * q, cy + ty * q)], colour)
    f.poly([(cx - q, cy - q), (cx + q, cy - q), (cx + q, cy + q), (cx - q, cy + q)], HULL)  # the front square
    w, h = q * 0.9, q * (0.26 if not small else 0.36)
    f.rr([cx - w, cy - h, cx + w, cy + h], 6, INK)                            # the docking slot
    if not small:
        f.rr([cx - w + 10, cy - h + 8, cx + w - 10, cy + h - 8], 4, d3.dark(SLOT, 0.55))
    if chosen:                                                                # chosen: a ship in the slot
        f.poly([(cx - w * 0.5, cy + h * 0.6), (cx, cy - h * 0.7), (cx + w * 0.5, cy + h * 0.6)], SHIP)
    else:
        f.rr([cx - w * 0.7, cy - 4, cx + w * 0.7, cy + 4], 3, SLOT)          # the slot's lights
    return 36


def draw_start(d3, f, small, chosen=False):
    """Oolite-Start: the station on a patch of space with two stars"""
    f.rr([24, 24, 488, 488], 60, SPACE, (40, 52, 80))
    if not small:
        for x, y in ((82, 90), (420, 400), (400, 96)):
            f.ell([x - 8, y - 8, x + 8, y + 8], (220, 228, 240))
    draw_station(d3, f, small, chosen, (256, 256), 180)
    return 34


def draw_installer(d3, f, small, chosen=False):
    """Install-Oolite: the parcel, open, the arrow bringing Oolite down into it; the station on its side"""
    blue = (59, 120, 216)
    f.poly([(60, 230), (256, 170), (452, 230), (256, 290)], d3.dark(CARD, 0.35))
    f.poly([(60, 230), (256, 290), (256, 490), (60, 420)], CARD)
    f.poly([(256, 290), (452, 230), (452, 420), (256, 490)], d3.dark(CARD, 0.18))
    f.poly([(60, 230), (20, 170), (216, 112), (256, 170)], d3.light(CARD, 0.25))
    f.poly([(452, 230), (492, 170), (296, 112), (256, 170)], d3.light(CARD, 0.12))
    f.poly([(92, 300), (224, 340), (224, 440), (92, 400)], SPACE)             # the label: the station
    draw_station(d3, f, True, False, (158, 370), 52)
    drop = 80 if chosen else 0
    w = 110 if not small else 140
    f.rr([256 - w // 2, 0 + drop, 256 + w // 2, 150 + drop], 16, blue, d3.light(blue, 0.4))
    f.poly([(256 - w - 60, 140 + drop), (256 + w + 60, 140 + drop), (256, 280 + drop)], d3.light(blue, 0.1))
    return 40


def frames_of(d3, draw, size=64):
    """both frames of a drawing, cut to one square, hard alpha, as soft3d_dosbox.frames_of"""
    from PIL import Image, ImageChops
    pair = []
    for chosen in (False, True):
        f = d3.Flat(512)
        thick = draw(d3, f, False, chosen)
        im = d3.solid(f.img, thick, 512, chosen)
        if chosen:
            up = Image.new("RGBA", (512, 512))
            up.alpha_composite(im, (0, 0), (0, 16))
            im = up
        pair.append(im)
    m = ImageChops.lighter(pair[0].getchannel("A"), pair[1].getchannel("A")).point(lambda v: 255 if v >= 60 else 0)
    x0, y0, x1, y1 = m.getbbox()
    side = int(max(x1 - x0, y1 - y0) * 1.06)
    cx, cy = (x0 + x1) // 2, (y0 + y1) // 2
    box = (cx - side // 2, cy - side // 2, cx - side // 2 + side, cy - side // 2 + side)
    return [d3.hard(p.crop(box), size) for p in pair]


def write(amiga_icon, path: Path, meta: dict, pics):
    """the icon at path: meta and the two frames"""
    out = []
    for im in pics:
        w, h = im.size
        out.append((w, h, amiga_icon.dither_rgb565(w, h, im.tobytes())))
    data = amiga_icon.write_os35(meta, out)
    back = amiga_icon.read_icon(data)
    if back["type"] != meta["type"] or len(back["imag"]) != 2:
        raise SystemExit(f"{path} does not read back as written")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    print(f"{path.relative_to(ROOT)}: {len(data)} bytes")


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--gfopenup", required=True, type=Path)
    ap.add_argument("--amicradle", required=True, type=Path)
    ap.add_argument("--sheet", type=Path, help="also a sheet of the four, both frames, light and dark, 4x")
    a = ap.parse_args(argv)
    sys.path.insert(0, str(a.gfopenup / "tools"))
    sys.path.insert(0, str(a.amicradle / "scripts" / "icons"))
    import amiga_icon                                                         # noqa: E402
    import soft3d_drives_art as d3                                            # noqa: E402
    import soft3d_kinds_art as k3                                             # noqa: E402

    # the badge first: port_installer.py reads it when it writes Install-Oolite
    f = d3.Flat(512)
    thick = draw_station(d3, f, False)
    d3.solid(f.img, thick, 512, False).resize((256, 256)).save(ROOT / "icons" / "art" / "oolite.png")
    print("icons/art/oolite.png")

    icons = {
        ROOT / "icons" / "Oolite.info": [k3.frames("drawer:rocketlaunch", 64, c) for c in (False, True)],
        ROOT / "icons" / "Oolite-Start.info": frames_of(d3, draw_start),
        ROOT / "icons" / "ReadMe.info": [k3.frames("project", 64, c) for c in (False, True)],
    }
    for path, pics in icons.items():
        write(amiga_icon, path, META[path.name], pics)
    inst = ROOT / "package" / "installer" / "Install-Oolite.info"
    if inst.exists():                       # port_installer.py's icon, its meta kept, repainted
        old = amiga_icon.read_icon(inst.read_bytes())
        meta = {k: old[k] for k in ("type", "defaultTool", "toolTypes", "toolWindow", "stack", "x", "y")}
        meta["drawer"] = None
        pics = frames_of(d3, draw_installer)
        write(amiga_icon, inst, meta, pics)
        icons[inst] = pics
    if a.sheet:
        from PIL import Image
        cell, gap = 64 * 4, 16
        sheet = Image.new("RGBA", (len(icons) * 2 * (cell + gap) + gap, 2 * (cell + gap) + gap))
        for row, bg in enumerate(((226, 230, 236, 255), (40, 44, 56, 255))):
            sheet.paste(bg, (0, row * (cell + gap), sheet.width, (row + 1) * (cell + gap) + gap))
            for i, pics in enumerate(icons.values()):
                for c, im in enumerate(pics):
                    big = im.resize((cell, cell), Image.NEAREST)
                    sheet.alpha_composite(big, (gap + (i * 2 + c) * (cell + gap), gap + row * (cell + gap)))
        sheet.save(a.sheet)
        print(f"sheet: {a.sheet}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
