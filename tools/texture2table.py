import pywavefront
import argparse
from pathlib import Path
from PIL import Image

parser = argparse.ArgumentParser(
                    prog='obj2c',
                    description='convert obj file to PL3D-KC object structure')

parser.add_argument('filename_in')
parser.add_argument('filename_out')
parser.add_argument('--name', '-n', default=None, type=str)
parser.add_argument('--width', '-wi', default=32, type=int, help="Texture width")
parser.add_argument('--height', '-he', default=32, type=int, help="Texture height")

args = parser.parse_args()

file_out = open(args.filename_out, "xt")

data_name = Path(args.filename_in).stem
if args.name is not None:
	data_name = args.name
data_texture_name = data_name

tex = Image.open(args.filename_in)
tex = tex.resize((args.width, args.height))
tex = tex.convert("L")

file_out.write("static const L3_COLORTYPE " +  data_texture_name + "_data[" + str(args.width * args.height) + "] = {\n")
for p in list(tex.getdata()):
	file_out.write(str(p) + ",")
file_out.write("};\n")

file_out.write("static const L3_Texture " + data_texture_name + "= {\n")
file_out.write(".width = " + str(args.width) + ",\n")
file_out.write(".height = " + str(args.height) + ",\n")
file_out.write(".data = " + data_texture_name + "_data,\n")
file_out.write("};\n")
