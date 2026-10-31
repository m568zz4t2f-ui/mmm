#!/usr/bin/env python3
"""Deterministic crops, exterior masks and resampling of recovered Higgsfield originals.
No pixels are painted, relit, reconstructed or taken from another generator.
"""
from pathlib import Path
import hashlib,json
from PIL import Image,ImageDraw,ImageChops
ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'assets/tide/deep-current-studio/originals'
DEST=SOURCE.parent/'runtime';DEST.mkdir(exist_ok=True)
MANIFEST=ROOT/'docs/deep-current-studio/asset-production-manifest.json'
# name, source prefix, reviewed crop, runtime pixels, source slice margins, logical corners
SPECS=[
 ('chassis','H01-chassis-main',(0,0,5024,3344),(1620,1080),[32,32,32,32],10),
 ('expanded','H01-chassis-expanded',(426,0,3142,4736),(1080,1440),[32,32,32,32],10),
 ('delay','H02-delay',(80,74,1964,1970),(1020,930),[42,42,42,42],12),
 ('reverb','H03-reverb',(80,74,1964,1970),(1020,930),[42,42,42,42],12),
 ('master','H04-master',(120,720,6170,1890),(1536,294),[40,40,40,40],8),
 ('knob','H05-universal',(655,663,1393,1389),(384,384),None,0),
 ('glass','H06-display',(248,171,2889,1141),(1422,435),[54,54,54,54],8),
 ('button_normal','H07-button-normal',(554,716,1815,1050),(336,90),[24,24,24,24],5),
 ('button_hover','H07-button-hover',(554,716,1815,1050),(336,90),[24,24,24,24],5),
 ('button_pressed','H07-button-pressed',(554,716,1815,1050),(336,90),[24,24,24,24],5),
 ('button_delay','H07-button-active-delay',(554,716,1815,1050),(336,90),[24,24,24,24],5),
 ('button_reverb','H07-button-active-reverb',(554,716,1815,1050),(336,90),[24,24,24,24],5),
 ('button_disabled','H07-button-disabled',(554,716,1815,1050),(336,90),[24,24,24,24],5),
 ('field_normal','H08-selector-normal-corrected',(107,744,2262,1028),(756,90),[24,24,24,24],5),
 ('field_hover','H08-selector-hover',(107,744,2262,1028),(756,90),[24,24,24,24],5),
 ('field_active','H08-selector-active',(107,744,2262,1028),(756,90),[24,24,24,24],5),
 ('field_disabled','H08-selector-disabled',(107,744,2262,1028),(756,90),[24,24,24,24],5),
 ('track','H09-linear-track',(323,636,2814,706),(900,24),[18,8,18,8],4),
 ('thumb','H09-linear-thumb',(815,360,1235,1604),(36,108),None,0),
 ('body','H10-callout-browser-body',(268,144,2277,1416),(1275,810),[72,72,72,72],8),
 ('selected_row','H10-browser-selected-row',(170,552,2960,712),(1284,72),[12,12,12,12],3),
 ('chevron','H11-dropdown',(620,777,1428,1262),(72,44),None,0)]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def run():
 manifest=json.loads(MANIFEST.read_text());exports=[]
 for name,prefix,box,size,slices,corner in SPECS:
  source=next(SOURCE.glob(prefix+'*.png'));im=Image.open(source).convert('RGBA');crop=im.crop(box)
  # Flood-fill only the dark exterior connected to the crop perimeter.
  # Enclosed dark screen wells remain opaque; no interior repaint or needle repair.
  mask_note='none; opaque full-surface source'
  if name not in ('chassis','expanded'):
   rgb=crop.convert('RGB');r,g,b=rgb.split();bright=ImageChops.lighter(ImageChops.lighter(r,g),b)
   outside=bright.point(lambda v:255 if v<32 else 0)
   for point in [(0,0),(crop.width-1,0),(0,crop.height-1),(crop.width-1,crop.height-1)]:
    if outside.getpixel(point)==255:ImageDraw.floodfill(outside,point,128,thresh=0)
   alpha=outside.point(lambda v:0 if v==128 else 255);crop.putalpha(alpha)
   mask_note='exterior flood mask at max(R,G,B)<32; enclosed interiors retained'
  image=crop.resize(size,Image.Resampling.LANCZOS)
  if name in ('chassis','expanded'):image=image.convert('RGB')
  target=DEST/(name+'.png');image.save(target,optimize=True)
  record=dict(name=name,source_path=str(source.relative_to(ROOT)),source_sha256=sha(source),source_dimensions=im.size,crop=list(box),mask=mask_note,export_path=str(target.relative_to(ROOT)),export_dimensions=list(size),export_sha256=sha(target),encoded_bytes=target.stat().st_size,decoded_bytes=image.width*image.height*len(image.getbands()),alpha_bounds=image.getchannel('A').getbbox() if image.mode=='RGBA' else [0,0,*size],slice_margins=slices,logical_corner=corner,approved=True)
  exports.append(record)
 for asset in manifest:
  matching=[e for e in exports if Path(e['source_path']).name.startswith(asset['id']+'-')]
  asset['exports']=matching;asset['approved']=bool(matching);asset['status']='prepared_approved' if matching else 'optional_omitted'
  asset['preparation_note']='Originals unchanged; reviewed full-resolution crops and native-size samples. See per-export masks, hashes and slice margins. Native text, pointers/arcs, focus and live plots remain runtime rendering.'
  for candidate in asset.get('recovered_candidates',[]):
   candidate['production_approved']=any(e['source_path']==candidate['source_path'] for e in matching)
  if asset['id']=='H08':asset['rejected_candidates']=[{'job_prefix':'05e6dca5','reason':'Pressed field interior is too busy; use approved normal field with native pressed/focus border. Earlier arrow-bearing normal attempts remain superseded.'}]
  if asset['id']=='H12':asset['preparation_note']='Optional contour omitted to keep decorative curves separate from live audio-state visualizations.'
 MANIFEST.write_text(json.dumps(manifest,indent=2)+'\n')
 summary=dict(provider='Higgsfield',material_master_job='b8504d01-4be3-41cb-b74f-2c3e574b3ff6',new_generations_submitted=0,exports=exports,total_decoded_bytes=sum(e['decoded_bytes'] for e in exports),total_encoded_bytes=sum(e['encoded_bytes'] for e in exports))
 (DEST/'manifest.json').write_text(json.dumps(summary,indent=2)+'\n')
 print(f"Prepared {len(exports)} exports; decoded {summary['total_decoded_bytes']/1048576:.2f} MiB")
if __name__=='__main__':run()
