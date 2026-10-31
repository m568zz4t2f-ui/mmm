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
 ('chassis','H01-chassis-main',(0,0,5024,3344),(1620,1080),[72,72,72,72],10),
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
 ('track','H09-linear-track',(323,636,2814,706),(900,24),[18,8,18,8],4),
 ('thumb','H09-linear-thumb',(815,360,1235,1604),(36,108),None,0),
 ('body','H10-callout-browser-body',(268,144,2277,1416),(1275,810),[72,72,72,72],8),
 ('selected_row','H10-browser-selected-row',(170,552,2960,712),(1284,72),[12,12,12,12],3),
 ('chevron','H11-dropdown',(620,777,1428,1262),(72,44),None,0)]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def run():
 manifest=json.loads(MANIFEST.read_text());exports=[]
 corrections=json.loads((ROOT/'docs/deep-current-studio/correction-artwork-receipt.json').read_text())['assets']
 original_receipt=json.loads((ROOT/'docs/deep-current-studio/recovered-artwork-receipt.json').read_text())['assets']
 known={a['source_path']:a for a in original_receipt+corrections}
 for name,prefix,box,size,slices,corner in SPECS:
  source=next(SOURCE.glob(prefix+'*.png'))
  correction=next((a for a in corrections if a['asset']==prefix[:3]),None)
  if name in ('chassis','delay','reverb') and correction:
   source=ROOT/correction['source_path']
   box=(268,144,2277,1416) if name=='chassis' else (78,73,1967,1975)
  im=Image.open(source).convert('RGBA');crop=im.crop(box)
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
  record=dict(name=name,job_id=known[str(source.relative_to(ROOT))]['job_id'],model_id='flux_3_image',intended_controls={'chassis':['main editor body','legacy expanded body'],'body':['modern machine pages','all browser scopes','menus','tooltips','quick guide','plugin-owned dialogs'],'knob':['all rotary sliders'],'glass':['stereo delay display','reverb display']}.get(name,[name.replace('_',' ')]),source_path=str(source.relative_to(ROOT)),source_sha256=sha(source),source_dimensions=im.size,crop=list(box),mask=mask_note,export_path=str(target.relative_to(ROOT)),export_dimensions=list(size),export_sha256=sha(target),encoded_bytes=target.stat().st_size,decoded_bytes=image.width*image.height*len(image.getbands()),alpha_bounds=image.getchannel('A').getbbox() if image.mode=='RGBA' else [0,0,*size],slice_margins=slices,logical_corner=corner,approved=True)
  exports.append(record)
 for asset in manifest:
  matching=[e for e in exports if Path(e['source_path']).name.startswith(asset['id']+'-')]
  asset['correction_candidates']=[dict(a,production_approved=any(e['source_path']==a['source_path'] for e in matching)) for a in corrections if a['asset']==asset['id']]
  if matching:
   primary=matching[0];asset.update(job_id=primary['job_id'],model_id='flux_3_image',source_dimensions=primary['source_dimensions'],source_sha256=primary['source_sha256'],crop=primary['crop'],slice_margins=primary['slice_margins'])
  asset['exports']=matching;asset['approved']=bool(matching);asset['status']='prepared_approved' if matching else 'optional_omitted'
  asset['preparation_note']='Originals unchanged; reviewed full-resolution crops and native-size samples. See per-export masks, hashes and slice margins. Native text, pointers/arcs, focus and live plots remain runtime rendering.'
  if asset['id']=='H08':
   asset['runtime_aliases']={'field_normal':'button_pressed','field_hover':'button_hover','field_active':'button_reverb','field_disabled':'button_disabled'}
   asset['approved']=True;asset['status']='approved_shared_inset_family'
   asset['preparation_note']='Recovered H08 interiors are too busy at native size. Use the accepted H07 shallow-inset family with native arrows and state/focus rendering. This reuses Higgsfield art without repainting any materials.'
  for candidate in asset.get('recovered_candidates',[]):
   candidate['production_approved']=any(e['source_path']==candidate['source_path'] for e in matching)
  if asset['id']=='H08':asset['rejected_candidates']=[{'family':'recovered H08 states','reason':'Interior texture competes with text at native control size; use shared accepted inset H07 skins. Earlier arrow-bearing attempts remain superseded.'}]
  if asset['id']=='H12':asset['preparation_note']='Optional contour omitted to keep decorative curves separate from live audio-state visualizations.'
 MANIFEST.write_text(json.dumps(manifest,indent=2)+'\n')
 summary=dict(provider='Higgsfield',material_master_job='b8504d01-4be3-41cb-b74f-2c3e574b3ff6',new_generations_submitted=4,exports=exports,total_decoded_bytes=sum(e['decoded_bytes'] for e in exports),total_encoded_bytes=sum(e['encoded_bytes'] for e in exports))
 summary['runtime_aliases']={'expanded':'chassis','field_normal':'button_pressed','field_hover':'button_hover','field_active':'button_reverb','field_disabled':'button_disabled'}
 (DEST/'manifest.json').write_text(json.dumps(summary,indent=2)+'\n')
 print(f"Prepared {len(exports)} exports; decoded {summary['total_decoded_bytes']/1048576:.2f} MiB")
if __name__=='__main__':run()
