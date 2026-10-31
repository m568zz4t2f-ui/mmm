#!/usr/bin/env python3
"""Compare three repeated 1/4/8/16-instance open/closed UI render stress fixtures."""
import subprocess,sys,ctypes,os,argparse
os.environ.setdefault("XDG_CONFIG_HOME","/workspace/work/tide-qa-config")
os.environ.setdefault("XDG_CACHE_HOME","/workspace/work/tide-qa-cache")
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

raise SystemExit(subprocess.call(sys.argv[1:]))
