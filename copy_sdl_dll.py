import os
import shutil
from pathlib import Path

def copy_sdl_dll():
    # Source path to SDL2.dll
    sdl_dll_src = r"C:\SDL2\x86_64-w64-mingw32\bin\SDL2.dll"
    
    # Destination path in the simulator build directory
    build_dir = Path(".pio/build/simulator")
    sdl_dll_dst = build_dir / "SDL2.dll"
    
    # Create build directory if it doesn't exist
    build_dir.mkdir(parents=True, exist_ok=True)
    
    # Copy the DLL
    if os.path.exists(sdl_dll_src):
        shutil.copy2(sdl_dll_src, str(sdl_dll_dst))
        print(f"Copied {sdl_dll_src} to {sdl_dll_dst}")
    else:
        print(f"Error: {sdl_dll_src} not found")

if __name__ == "__main__":
    copy_sdl_dll()
