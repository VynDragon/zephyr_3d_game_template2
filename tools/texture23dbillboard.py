import pywavefront
import argparse
from pathlib import Path
from PIL import Image

DEFAULT_SET = [
	"Right",
	"Front",
	"Left",
	"Back",
	"p45_Right",
	"p45_Front",
	"p45_Left",
	"p45_Back",
]

parser = argparse.ArgumentParser(
                    prog='texture23dbillboard',
                    description='convert set of image files to 3d billboard header.')

parser.add_argument('folder_path', type=Path)
parser.add_argument('filename_out', type=Path)
parser.add_argument('width', type=int)
parser.add_argument('height', type=int)
parser.add_argument('--scale', '-s', default=512, type=int)
parser.add_argument('--name', '-n', default=None, type=str)
parser.add_argument('--pattern', '-p', default="{0:04d}_{1:s}.png", type=str, help="pattern to match frame images")
parser.add_argument('--interval', '-i', default=8, type=int, help="interval between frame numbers")
parser.add_argument('--set', '-st', default=None, type=Path, help="dataset to apply to pattern past the frame count id 0")

args = parser.parse_args()

file_out = open(args.filename_out, "wt")

data_name = Path(args.folder_path).stem
if args.name is not None:
	data_name = args.name
data_texture_name = data_name + "_texture"
data_bb_name = data_name + "_billboard"

pattern_set = DEFAULT_SET

if args.set is not None:
	with open(args.set, "rt") as set_file:
		pattern_set = json.load(set_file)

frame_index = 0

while True:
	set_textures = []
	for set_i in pattern_set:
		image_path = args.folder_path / args.pattern.format(frame_index, set_i)

		if not image_path.is_file():
			if frame_index == 0:
				raise ValueError("Not a frame set")
			file_out.write("static const L3_Billboard_3D *" +  data_name + "_frames[] = {\n")
			for p in range(0, frame_index, args.interval):
				file_out.write("&" + data_bb_name + "_" + str(p) + ",\n")
			file_out.write("};\n")
			file_out.write("static const L3_Object " +  data_name + " = {\n")
			file_out.write(".transform.scale.x = L3_F,\n")
			file_out.write(".transform.scale.y = L3_F,\n")
			file_out.write(".transform.scale.z = L3_F,\n")
			file_out.write(".transform.scale.w = 0,\n")
			file_out.write(".transform.translation.x = 0,\n")
			file_out.write(".transform.translation.y = 0.825*L3_F,\n")
			file_out.write(".transform.translation.z = 0,\n")
			file_out.write(".transform.translation.w = L3_F,\n")
			file_out.write(".transform.rotation.x = 0,\n")
			file_out.write(".transform.rotation.y = 0,\n")
			file_out.write(".transform.rotation.z = 0,\n")
			file_out.write(".transform.rotation.w = L3_F,\n")
			file_out.write(".config.backfaceCulling = 0,\n")
			file_out.write(".solid_color = 0xff,\n")
			file_out.write(".billboard_3D = &" + data_bb_name + "_0" + ",\n")
			file_out.write(".config.visible = L3_VISIBLE_BILLBOARD | L3_VISIBLE_BILLBOARD_ZSLOW | L3_VISIBLE_BILLBOARD_FRONTIFY | L3_VISIBLE_BILLBOARD_3D,\n")
			file_out.write("};\n")
			file_out.close()
			exit(0)

		print("Processing {}...".format(image_path))

		tex = Image.open(image_path)
		tex = tex.resize((args.width, args.height))
		tex = tex.convert("L")

		data_texture_name_here = data_texture_name + "_" + str(frame_index) + "_" + set_i
		set_textures.append(data_texture_name_here)

		file_out.write("static const L3_COLORTYPE " +  data_texture_name_here + "_data[" + str(args.width * args.height) + "] = {\n")
		for p in list(tex.getdata()):
			file_out.write(str(p) + ",")
		file_out.write("};\n")

		file_out.write("static const L3_Texture " + data_texture_name_here + " = {\n")
		file_out.write(".width = " + str(args.width) + ",\n")
		file_out.write(".height = " + str(args.height) + ",\n")
		file_out.write(".data = " + data_texture_name_here + "_data,\n")
		file_out.write("};\n")

	data_bb_name_here = data_bb_name + "_" + str(frame_index)

	file_out.write("static const L3_Texture *" + data_bb_name_here + "_textures[] = {\n")

	for set_t_i in set_textures:
		file_out.write("&" + set_t_i + ",\n")

	file_out.write("};\n")

	file_out.write("static const L3_Billboard_3D " +  data_bb_name_here + " = {\n")
	file_out.write(".textures = " + data_bb_name_here + "_textures,\n")
	file_out.write(".texture_cnt = {},\n".format(len(set_textures)))
	file_out.write(".y_cnt = 4,\n")
	file_out.write(".xz_cnt = 8,\n")
	file_out.write(".scale = " + str(args.scale) + ",\n")
	file_out.write(".transparency.transparency = 0xff,\n")
	file_out.write(".transparency.transparency_threshold = 40,\n")
	file_out.write("};\n")

	frame_index = frame_index + args.interval
