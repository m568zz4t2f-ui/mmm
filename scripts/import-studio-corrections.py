#!/usr/bin/env python3
"""Import only explicitly recorded, completed Higgsfield correction jobs."""
import hashlib,io,json,urllib.parse,urllib.request
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
DOCS=ROOT/'docs/deep-current-studio'
manifest=json.loads((DOCS/'correction-jobs.json').read_text())
freeze=json.loads((DOCS/'baseline-freeze.json').read_text())
for path,digest in freeze['protected_source_and_presets'].items():
 if hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=digest:raise ValueError('Protected source changed: '+path)
records=[]
for job in manifest['jobs']:
 if job.get('status')!='completed':continue
 job_id=job['submission']['results'][0]['id'];url=urllib.parse.urlsplit(job['result_url'])
 if url.scheme!='https' or url.hostname!='d8j0ntlcm91z4.cloudfront.net' or url.username or url.password:raise ValueError('Unexpected correction source')
 target=ROOT/'assets/tide/deep-current-studio/originals/corrections'/(job['asset']+'-'+job_id+'.png')
 if target.is_file():data=target.read_bytes()
 else:
  with urllib.request.urlopen(job['result_url'],timeout=90) as response:data=response.read(64*1024*1024+1)
 if len(data)>64*1024*1024:raise ValueError('Oversized correction')
 with Image.open(io.BytesIO(data)) as image:
  if image.format!='PNG' or image.width*image.height>40_000_000:raise ValueError('Invalid correction image')
  dimensions=list(image.size);image.verify()
 digest=hashlib.sha256(data).hexdigest();target.parent.mkdir(parents=True,exist_ok=True)
 if not target.exists():target.write_bytes(data)
 records.append(dict(asset=job['asset'],job_id=job_id,source_path=str(target.relative_to(ROOT)),source_url=job['result_url'],source_sha256=digest,source_dimensions=dimensions,encoded_bytes=len(data),approved=False))
receipt=DOCS/'correction-artwork-receipt.json'
if receipt.exists():
 old={r['source_path']:r['source_sha256'] for r in json.loads(receipt.read_text())['assets']}
 for r in records:
  if r['source_path'] in old and old[r['source_path']]!=r['source_sha256']:raise ValueError('Correction original changed')
receipt.write_text(json.dumps(dict(provider='Higgsfield',assets=records),indent=2)+'\n')
print('Verified/imported',len(records),'completed corrections; protected source unchanged.')
