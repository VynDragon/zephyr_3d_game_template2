import pywavefront
import argparse
from pathlib import Path
from PIL import Image

parser = argparse.ArgumentParser(
                    prog='texture2billboard',
                    description='convert image file to billboard header.')

parser.add_argument('filename_in')
parser.add_argument('filename_out')
parser.add_argument('width', type=int)
parser.add_argument('height', type=int)
parser.add_argument('--scale', '-s', default=512, type=int)
parser.add_argument('--name', '-n', default=None, type=str)

args = parser.parse_args()

file_out = open(args.filename_out, "xt")

data_name = Path(args.filename_in).stem
if args.name is not None:
	data_name = args.name
data_texture_name = data_name + "_texture"
data_bb_name = data_name + "_billboard"

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

file_out.write("static const L3_Billboard " +  data_bb_name + " = {\n")
file_out.write(".texture = &" + data_texture_name + ",\n")
file_out.write(".scale = " + str(args.scale) + ",\n")
file_out.write("};\n")

file_out.write("static const L3_Object " +  data_name + " = {\n")
file_out.write(".transform.scale.x = L3_F,\n")
file_out.write(".transform.scale.y = L3_F,\n")
file_out.write(".transform.scale.z = L3_F,\n")
file_out.write(".transform.scale.w = 0,\n")
file_out.write(".transform.translation.x = 0,\n")
file_out.write(".transform.translation.y = 0,\n")
file_out.write(".transform.translation.z = 0,\n")
file_out.write(".transform.translation.w = L3_F,\n")
file_out.write(".transform.rotation.x = 0,\n")
file_out.write(".transform.rotation.y = 0,\n")
file_out.write(".transform.rotation.z = 0,\n")
file_out.write(".transform.rotation.w = L3_F,\n")
file_out.write(".billboard = &" + data_bb_name + ",\n")
file_out.write(".config.visible = L3_VISIBLE_BILLBOARD,\n")
file_out.write("};\n")
