"""Draw original project artwork; no game resources are read."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parents[1]
image=Image.new('RGB',(1024,1024),(12,24,40));draw=ImageDraw.Draw(image)
def font(size):
    return ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',size)
for y in range(1024):
    draw.line((0,y,1024,y),fill=(12+y//100,24+y//70,40+y//40))
for x,y,r in [(94,97,2),(876,167,3),(170,365,2),(842,403,2),(920,761,2),(120,814,3)]:
    draw.ellipse((x-r,y-r,x+r,y+r),fill=(98,164,210))
draw.rounded_rectangle((68,68,956,956),radius=44,outline=(60,101,135),width=2)
draw.text((112,117),'CursorBridge',font=font(88),fill=(235,246,255))
draw.text((118,226),'STELLARIS EDITION',font=font(30),fill=(111,198,255))
draw.text((111,332),'1—96',font=font(178),fill=(235,246,255))
draw.text((702,464),'px',font=font(52),fill=(111,198,255))
draw.rounded_rectangle((122,601,881,613),radius=6,fill=(54,80,111))
draw.rounded_rectangle((122,601,513,613),radius=6,fill=(111,198,255))
draw.ellipse((495,589,531,625),fill=(111,198,255))
# An original simple geometric cursor motif, not extracted Stellaris art.
draw.polygon([(720,644),(720,768),(752,739),(774,785),(799,773),(777,729),(821,727)],fill=(235,246,255))
draw.text((118,688),'LIVE CURSOR SIZING',font=font(38),fill=(235,246,255))
draw.text((118,771),'WINDOWS x64  ·  OPEN SOURCE',font=font(29),fill=(157,191,215))
draw.rounded_rectangle((117,847,901,914),radius=16,fill=(39,74,106))
draw.text((143,857),'COMPANION APP REQUIRED',font=font(38),fill=(235,246,255))
target=root/'docs/images/cover.png';target.parent.mkdir(parents=True,exist_ok=True)
image.save(target,optimize=True)
(root/'workshop/preview.png').write_bytes(target.read_bytes())
print('Original 1024×1024 cover and Workshop preview created.')
