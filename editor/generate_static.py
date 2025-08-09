import argparse
from pathlib import Path
import shutil
import glob
import os
import json

parser = argparse.ArgumentParser(
                    prog='generate objects',
                    description='use tools and configuration to generate data')

parser.add_argument('generated_folder', help="typically build/generated")
parser.add_argument('input_json')
parser.add_argument('output_folder')
parser.add_argument('--name', '-n', default="map.h", type=str)

args = parser.parse_args()

if not Path(args.output_folder).exists() :
	Path(args.output_folder).mkdir()

for header in glob.glob(os.path.join(glob.escape(args.generated_folder), "**/*.h"), recursive=True):
	header_path = Path(header).relative_to(Path(args.generated_folder))
	if header_path.stem != "generated_objects" and header_path.stem != "objects":
		out_path = Path(args.output_folder).joinpath(header_path)
		if not out_path.parent.exists() :
			out_path.parent.mkdir(parents=True)
		shutil.copyfile(header, out_path)

with open(Path(args.output_folder).joinpath(args.name), "wt") as static_file:
	with open(Path(args.generated_folder).joinpath(Path("objects.h")), "rt") as objects_header:
		static_file.write(objects_header.read())
	static_file.write("static const Engine_Object " + Path(args.name).stem + "[] = {\n")
	with open(args.input_json, "rt") as conf_file:
		conf = json.load(conf_file)
		for i, obj in enumerate(conf):
			static_file.write("\t{\n")
			static_file.write("\t\t.visual.transform.scale.x = " + str(obj["transform"]["scale"]["x"]) + ",\n")
			static_file.write("\t\t.visual.transform.scale.y = " + str(obj["transform"]["scale"]["y"]) + ",\n")
			static_file.write("\t\t.visual.transform.scale.z = " + str(obj["transform"]["scale"]["z"]) + ",\n")
			static_file.write("\t\t.visual.transform.scale.w = " + str(obj["transform"]["scale"]["w"]) + ",\n")
			static_file.write("\t\t.visual.transform.translation.x = " + str(obj["transform"]["translation"]["x"]) + ",\n")
			static_file.write("\t\t.visual.transform.translation.y = " + str(obj["transform"]["translation"]["y"]) + ",\n")
			static_file.write("\t\t.visual.transform.translation.z = " + str(obj["transform"]["translation"]["z"]) + ",\n")
			static_file.write("\t\t.visual.transform.translation.w = " + str(obj["transform"]["translation"]["w"]) + ",\n")
			static_file.write("\t\t.visual.transform.rotation.x = " + str(obj["transform"]["rotation"]["x"]) + ",\n")
			static_file.write("\t\t.visual.transform.rotation.y = " + str(obj["transform"]["rotation"]["y"]) + ",\n")
			static_file.write("\t\t.visual.transform.rotation.z = " + str(obj["transform"]["rotation"]["z"]) + ",\n")
			static_file.write("\t\t.visual.transform.rotation.w = " + str(obj["transform"]["rotation"]["w"]) + ",\n")
			static_file.write("\t\t.visual.config.backfaceCulling = " + str(obj["backfaceCulling"]) + ",\n")
			static_file.write("\t\t.visual.config.visible = " + str(obj["visible"]) + ",\n")
			if obj["visible"] & (1 << 4):
				static_file.write("\t\t.visual.billboard = &" + str(obj["visual_i"]) + ",\n")
			else:
				static_file.write("\t\t.visual.model = &" + str(obj["visual_i"]) + ",\n")
			static_file.write("\t\t.visual_type = " + str(obj["visual_type"]) + ",\n")
			static_file.write("\t\t.view_range = " + str(obj["view_range"]) + ",\n")
			if len(obj["collisions"]) > 0:
				static_file.write("\t\t.collisions = &" + str(obj["collisions"]) + ",\n")
			else:
				static_file.write("\t\t.collisions = 0,\n")
			static_file.write("\t\t.process = 0,\n")
			static_file.write("\t\t.data = 0,\n")
			static_file.write("\t},\n")
	static_file.write("};\n")
