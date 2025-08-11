import argparse
from pathlib import Path
import os
import glob
import json

def generate_obj_header(tool_path, obj_path):
	rel_path = Path(obj_path).relative_to(Path(args.data_folder))
	out_name = os.path.join(args.output_folder, rel_path.parent.joinpath(rel_path.stem))
	out_real_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
	if not Path(args.output_folder).joinpath(rel_path.parent).exists() :
		Path(args.output_folder).joinpath(rel_path.parent).mkdir(parents=True)
	try:
		os.remove(out_name + ".h")
	except:
		pass
	return os.system("python3 " + tool_path + " " + obj_path + " " + out_name + ".h" + " -n " + out_real_name)

def generate_billboard_header(tool_path, bill_path, width: int, height: int):
	rel_path = Path(bill_path).relative_to(Path(args.data_folder))
	out_name = os.path.join(args.output_folder, rel_path.parent.joinpath(rel_path.stem))
	out_real_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
	if not Path(args.output_folder).joinpath(rel_path.parent).exists() :
		Path(args.output_folder).joinpath(rel_path.parent).mkdir(parents=True)
	return os.system("python3 " + tool_path + " " + bill_path + " " + out_name + ".h" + " -n " + out_real_name + " " + str(width) + " " + str(height))

def generate_obj(obj_path, col_file, output_path, obj_name):
	rel_path = Path(obj_path).relative_to(Path(args.data_folder))
	model_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
	#model_name = Path(obj_path).stem
	with open(output_path, "tx") as file_out:
		file_out.write("#pragma once\n")
		file_out.write("#ifndef ARRAY_SIZE\n#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))\n#endif\n")

		file_out.write("static const E_Collider " + obj_name + "_colliders[] = {\n")
		file_out.write(col_file.read())
		file_out.write("};\n")

		file_out.write("static const Engine_Collisions " + obj_name + "_collisions = {\n")
		file_out.write(".colliders = " + obj_name + "_colliders,\n")
		file_out.write(".colliderCount = ARRAY_SIZE(" + obj_name + "_colliders),\n")
		file_out.write("};\n")

		file_out.write("static const Engine_Object " +  obj_name + " = {\n")
		file_out.write(".visual = " + model_name + ",\n")
		file_out.write(".collisions = &" + obj_name + "_collisions,\n")
		file_out.write(".visual_type = ENGINE_VISUAL_MODEL,\n")
		file_out.write(".view_range = 65536,\n")
		file_out.write("};\n")

def generate_obj_nocol(obj_path, output_path, obj_name):
	rel_path = Path(obj_path).relative_to(Path(args.data_folder))
	model_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
	#model_name = Path(obj_path).stem
	with open(output_path, "tx") as file_out:
		file_out.write("#pragma once\n")
		file_out.write("#ifndef ARRAY_SIZE\n#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))\n#endif\n")

		file_out.write("static const Engine_Object " +  obj_name + " = {\n")
		file_out.write(".visual = " + model_name + ",\n")
		file_out.write(".collisions = NULL,\n")
		file_out.write(".visual_type = ENGINE_VISUAL_MODEL,\n")
		file_out.write(".view_range = 65536,\n")
		file_out.write("};\n")

def generate_billboard(obj_path, output_path, obj_name):
	rel_path = Path(obj_path).relative_to(Path(args.data_folder))
	model_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
	#model_name = Path(obj_path).stem
	with open(output_path, "tx") as file_out:
		file_out.write("#pragma once\n")
		file_out.write("#ifndef ARRAY_SIZE\n#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))\n#endif\n")

		file_out.write("static const Engine_Object " +  obj_name + " = {\n")
		file_out.write(".visual = " + model_name + ",\n")
		file_out.write(".collisions = NULL,\n")
		file_out.write(".visual_type = ENGINE_VISUAL_BILLBOARD,\n")
		file_out.write(".view_range = 65536,\n")
		file_out.write("};\n")

parser = argparse.ArgumentParser(
                    prog='generate objects',
                    description='use tools and configuration to generate data')

parser.add_argument('tools_path')
parser.add_argument('data_folder')
parser.add_argument('output_folder')

args = parser.parse_args()

index_header = open(os.path.join(args.output_folder, "generated_objects.h"), "wt")
index_header.write("#pragma once\n")
list_header = open(os.path.join(args.output_folder, "objects.h"), "wt")
list_header.write("#pragma once\n")
obj_names = []
collision_names = []
visual_names = []

for obj_path in glob.glob(os.path.join(glob.escape(args.data_folder), "**/*.obj"), recursive=True):
	rel_path = Path(obj_path).relative_to(Path(args.data_folder))
	obj_path_name = str(rel_path.parent.joinpath(rel_path.stem))
	if generate_obj_header(os.path.join(args.tools_path, "obj2ctexture.py"), obj_path) != 0:
		generate_obj_header(os.path.join(args.tools_path, "obj2c.py"), obj_path)
	index_header.write("#include \"" + obj_path_name + ".h\"\n")
	list_header.write("#include \"" + obj_path_name + ".h\"\n")

for conf_path in glob.glob(os.path.join(glob.escape(args.data_folder), "**/*.json"), recursive=True):
	rel_path = Path(conf_path).relative_to(Path(args.data_folder))
	conf_name = str(rel_path.parent.joinpath(rel_path.stem))
	conf_name_es = conf_name.replace('/','_')
	with open(conf_path, "rt") as conf_file:
		conf = json.load(conf_file)
		output_path = os.path.join(args.output_folder, conf_name + ".h")
		if not Path(output_path).parent.exists():
			Path(output_path).parent.mkdir(parents=True)
		if conf.get("obj") != None:
			if conf.get("col") != None:
				with open(os.path.join(glob.escape(args.data_folder), conf["col"]), "rt") as col_file:
					generate_obj(os.path.join(glob.escape(args.data_folder), conf["obj"]), col_file, output_path, conf_name_es)
			else:
				generate_obj_nocol(os.path.join(glob.escape(args.data_folder), conf["obj"]), output_path, conf_name_es)
		elif conf.get("billboard") != None:
			bill_path = os.path.join(glob.escape(args.data_folder), conf["billboard"])
			generate_billboard_header(os.path.join(args.tools_path, "texture2billboard.py"), bill_path, conf["width"], conf["height"])
			rel_path = Path(bill_path).relative_to(Path(args.data_folder))
			bill_path_name = str(rel_path.parent.joinpath(rel_path.stem))
			index_header.write("#include \"" + bill_path_name + ".h\"\n")
			list_header.write("#include \"" + bill_path_name + ".h\"\n")
			generate_billboard(bill_path, output_path, conf_name_es)
	obj_names.append(conf_name_es)
	if conf.get("col") != None:
		collision_names.append(conf_name_es + "_collisions")
	else:
		collision_names.append("")
	if conf.get("obj") != None:
		rel_path = Path(os.path.join(glob.escape(args.data_folder), conf["obj"])).relative_to(Path(args.data_folder))
		out_real_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
		visual_names.append(out_real_name + "_model")
	elif conf.get("billboard") != None:
		rel_path = Path(os.path.join(glob.escape(args.data_folder), conf["billboard"])).relative_to(Path(args.data_folder))
		out_real_name = str(rel_path.parent.joinpath(rel_path.stem)).replace('/','_')
		visual_names.append(out_real_name + "_billboard")
	else:
		visual_names.append("")
	index_header.write("#include \"" + conf_name + ".h\"\n")
	list_header.write("#include \"" + conf_name + ".h\"\n")



index_header.write("static const Engine_Object generated_object_list[] = {\n")
for name in obj_names:
	index_header.write(name + ",\n")
index_header.write("};\n")

index_header.write("static const char *generated_object_list_names[] = {\n")
for name in obj_names:
	index_header.write("\"" + name + "\",\n")
index_header.write("};\n")

index_header.write("static const char *generated_object_visual_names[] = {\n")
for name in visual_names:
	index_header.write("\"" + name + "\",\n")
index_header.write("};\n")

index_header.write("static const char *generated_object_collisions_names[] = {\n")
for name in collision_names:
	index_header.write("\"" + name + "\",\n")
index_header.write("};\n")
