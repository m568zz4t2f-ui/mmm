#!/usr/bin/env python3
"""Curate supplied preview materials; no generated controls or lettering ship."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageChops
import hashlib, json
root=Path(__file__).resolve().parents[1]
src=root/'assets/tide/deep-current/previews'; dest=src.parent/'production';dest.mkdir(exist_ok=True)
records=[]
def save(name, image, source, crop):
    path=dest/(name+'.png');image.save(path,optimize=True)
    records.append(dict(name=path.name,source=source,crop=crop,size=image.size,encoded_bytes=path.stat().st_size,decoded_bytes=image.width*image.height*len(image.getbands()),source_sha256=hashlib.sha256((src/source).read_bytes()).hexdigest()))
# Opaque, unlettered interior patches, excluding the generated controls and recess edges.
for name,source,crop in [('graphite','Tide_02.png',(180,590,420,900)),('teal','Tide_03.png',(110,680,420,920)),('chassis','Tide_01.jpg',(150,130,1050,680)),('glass','Tide_08.png',(250,235,950,280))]:
    patch=Image.open(src/source).crop(crop).convert('RGB')
    save(name,patch.resize((256,256),Image.Resampling.LANCZOS),source,crop)
# Remove the fixed painted needle with the adjacent unmarked material.
source='Tide_05.png'; face=Image.open(src/source).convert('RGBA')
mask=Image.new('L',face.size);ImageDraw.Draw(mask).rounded_rectangle((570,196,626,415),radius=15,fill=255)
mask=mask.filter(ImageFilter.GaussianBlur(4))
adjacent=ImageChops.offset(face,65,0)
face=Image.composite(adjacent,face,mask)
# Trim speckled alpha outside the actual circular silhouette. Keep the rim intact.
alpha=Image.new('L',face.size);ImageDraw.Draw(alpha).ellipse((122,98,1064,1048),fill=255)
face.putalpha(alpha)
face=face.crop((116,92,1070,1054)).resize((256,256),Image.Resampling.LANCZOS)
save('titanium',face,source,(116,92,1070,1054))
(dest/'manifest.json').write_text(json.dumps({'input_note':'Repository previews are 1200px derivatives, not the original full resolution images.','knob_repair':'Fixed needle reconstructed from adjacent face; outer alpha replaced with clean ellipse.','assets':records},indent=2)+'\n')
