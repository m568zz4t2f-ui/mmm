#!/usr/bin/env python3
"""Compare three repeated 1/4/8/16-instance open/closed UI render stress fixtures."""
import subprocess,sys,ctypes,os,argparse,tempfile,threading
# A bare Xvfb has no window manager to register the standard WM protocol atoms.
# Keep this connection alive so they survive between child benchmark processes.
if sys.platform.startswith('linux'):
 x=ctypes.CDLL('libX11.so.6');x.XOpenDisplay.argtypes=[ctypes.c_char_p];x.XOpenDisplay.restype=ctypes.c_void_p
 x.XInternAtom.argtypes=[ctypes.c_void_p,ctypes.c_char_p,ctypes.c_int];x.XInternAtom.restype=ctypes.c_ulong
 x.XSync.argtypes=[ctypes.c_void_p,ctypes.c_int]
 display=x.XOpenDisplay(os.environ['DISPLAY'].encode());assert display
 for name in ['WM_PROTOCOLS','WM_TAKE_FOCUS','WM_DELETE_WINDOW','_NET_WM_PING','WM_CHANGE_STATE','WM_STATE','_NET_WM_WINDOW_TYPE','_NET_WM_WINDOW_TYPE_NORMAL','_NET_WM_WINDOW_TYPE_COMBO','_NET_WM_STATE','_NET_WM_STATE_HIDDEN']:
  x.XInternAtom(display,name.encode(),False)
 x.XSync(display,False)
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('binary');parser.add_argument('output')
parser.add_argument('--baseline');parser.add_argument('--baseline-output')
parser.add_argument('--instances',type=int,nargs='+',choices=[1,4,8,16],default=[1,4,8,16])
parser.add_argument('--normal',action='store_true',help='Use natural dirty-region window events, without forced full paints.')
args=parser.parse_args()
if bool(args.baseline)!=bool(args.baseline_output):parser.error('supply both baseline options')
def measured_run(command):
 # POSIX wait4 gives this child's peak RSS rather than a cumulative process maximum.
 with tempfile.TemporaryFile(mode='w+t') as log:
  process=subprocess.Popen(command,stdout=log,stderr=subprocess.STDOUT)
  timeout=threading.Timer(90,process.kill);timeout.start()
  try:_,status,usage=os.wait4(process.pid,0)
  finally:timeout.cancel()
  process.returncode=os.waitstatus_to_exitcode(status);log.seek(0);output=log.read()
  if process.returncode:raise RuntimeError(output)
  rss=usage.ru_maxrss/(1024 if sys.platform=='darwin' else 1)
  return output.strip().splitlines()[-1],int(rss)

binaries=[args.binary]+([args.baseline] if args.baseline else [])
streams=[open(name,'w') for name in [args.output]+([args.baseline_output] if args.baseline else [])]
try:
 for stream in streams:
  stream.write('instances,state,elapsed_ms,process_cpu_ms,audio_batch_mean_ms,audio_batch_max_ms,forced_paint_mean_ms,event_lag_max_ms,trial,peak_rss_kib,mode\n');stream.flush()
 for count in args.instances:
  for opened in [0,1]:
   for trial in range(3):
    for version in (range(len(binaries)) if trial%2==0 else reversed(range(len(binaries)))):
     mode='normal' if args.normal else 'forced'
     line,rss=measured_run([binaries[version],'--bench-normal' if args.normal else '--bench',str(count),str(opened)])
     streams[version].write(line+','+str(trial+1)+','+str(rss)+','+mode+'\n');streams[version].flush()
finally:
 for stream in streams:stream.close()
