import pywavefront
import argparse
from pathlib import Path
import os
import glob
import json
from PIL import Image

MODEL_SCALE = 512
MUL_UV = 1

def generate_model_header(name, model_path, texture_name, texture_width, texture_height, out_path, notexture=False):
	data_vertices_name = name + "_vertices"
	data_index_name = name + "_indexes"
	data_texture_name = name + "_textures"
	data_uv_name = name + "_UVs"
	data_texindex_name = name + "_indexes_texture"
	data_model_name = name

	total_polys = 0

	object_in = pywavefront.Wavefront(model_path, collect_faces=True)
	file_out = open(out_path, "xt")

	file_out.write("#pragma once\nstatic const L3_Unit " +  data_vertices_name + "[] = {\n")
	for count, vertex in enumerate(object_in.vertices):
		x = int(vertex[0] * MODEL_SCALE)
		y = int(vertex[1] * MODEL_SCALE)
		z = int(vertex[2] * MODEL_SCALE)
		file_out.write(str(x) + "," + str(y) + "," + str(z) + ",\n")
	file_out.write("};\n")

	file_out.write("static const L3_Index " +  data_index_name + "[] = {\n")
	for mesh in object_in.mesh_list:
		total_polys = total_polys + len(mesh.faces)
		for face in mesh.faces:
			file_out.write(str(face[0]) + ", " + str(face[1]) + ", " + str(face[2]) + ",\n")
	file_out.write("};\n")

	if not notexture:

		file_out.write("static const L3_Texture *" +  data_texture_name + "[] = {\n")
		file_out.write("&" + texture_name + ",\n")
		file_out.write("};\n")

		file_out.write("static const L3_Unit " +  data_uv_name + "[] = {\n")
		for mesh in object_in.mesh_list:
			for faceid, face in enumerate(mesh.faces):
				face0uv = (mesh.materials[0].vertices[faceid * 5 * 3], mesh.materials[0].vertices[faceid * 5 * 3+ 1])
				face1uv = (mesh.materials[0].vertices[faceid * 5 * 3 + 5], mesh.materials[0].vertices[faceid * 5 * 3 + 6])
				face2uv = (mesh.materials[0].vertices[faceid * 5 * 3 + 10], mesh.materials[0].vertices[faceid * 5 * 3 + 11])
				# with fix for UV being the wrong way for some reason
				file_out.write(str(int(face0uv[0] * texture_width * MUL_UV)) + ", " + str(int(texture_height * MUL_UV - face0uv[1] * texture_height * MUL_UV)) + ",\n")
				file_out.write(str(int(face1uv[0] * texture_width * MUL_UV)) + ", " + str(int(texture_height * MUL_UV - face1uv[1] * texture_height * MUL_UV)) + ",\n")
				file_out.write(str(int(face2uv[0] * texture_width * MUL_UV)) + ", " + str(int(texture_height * MUL_UV - face2uv[1] * texture_height * MUL_UV)) + ",\n")
		file_out.write("};\n")

		file_out.write("static const L3_Index " +  data_texindex_name + "[] = {\n")
		for mesh in object_in.mesh_list:
			for faceid, face in enumerate(mesh.faces):
				if mesh.materials[0].name:
					file_out.write(str(0) + ",")
				else:
					file_out.write(str(-1) + ",")
		file_out.write("};\n")

	file_out.write("static const L3_Model3D " +  data_model_name + " = {\n")
	file_out.write(".vertices = " + data_vertices_name + ",\n")
	file_out.write(".triangleCount = " + str(int(total_polys)) + ",\n")
	file_out.write(".vertexCount = " + str(len(object_in.vertices)) + ",\n")
	file_out.write(".triangles = " + data_index_name + ",\n")
	if not notexture:
		file_out.write(".triangleTextures = " + data_texture_name + ",\n")
		file_out.write(".triangleUVs = " + data_uv_name + ",\n")
		file_out.write(".triangleTextureIndex = " + data_texindex_name + ",\n")
	else:
		file_out.write(".triangleTextures = NULL,\n")
		file_out.write(".triangleUVs = NULL,\n")
		file_out.write(".triangleTextureIndex = NULL,\n")
	file_out.write("};\n")
	file_out.close()

def generate_billboard_header(tool_path, name, bill_path, out_path, width: int, height: int):
	return os.system("python3 " + tool_path + " " + str(bill_path) + " " + str(out_path) + ".h" + " -n " + name + " " + str(width) + " " + str(height))

def generate_billboard(name, output_path, obj_name):
	with open(output_path, "ta+") as file_out:
		file_out.write("#pragma once\n")
		file_out.write("#ifndef ARRAY_SIZE\n#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))\n#endif\n")

		file_out.write("static const Engine_Object " +  obj_name + " = {\n")
		file_out.write(".visual = " + name + ",\n")
		file_out.write(".collisions = NULL,\n")
		file_out.write(".visual_type = ENGINE_VISUAL_BILLBOARD,\n")
		file_out.write(".view_range = 65536,\n")
		file_out.write("};\n")

def generate_object_header(name, model_name, output_path, textured = False, col_file=None):
	with open(output_path, "ta+") as file_out:
		file_out.write("#pragma once\n")
		file_out.write("#ifndef ARRAY_SIZE\n#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))\n#endif\n")

		if col_file != None:
			file_out.write("static const E_Collider " + name + "_colliders[] = {\n")
			file_out.write(col_file.read())
			file_out.write("};\n")

			file_out.write("static const Engine_Collisions " + name + "_collisions = {\n")
			file_out.write(".colliders = " + name + "_colliders,\n")
			file_out.write(".colliderCount = ARRAY_SIZE(" + name + "_colliders),\n")
			file_out.write("};\n")

		file_out.write("static const L3_Object " +  name + "_object = {\n")
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
		file_out.write(".config.backfaceCulling = 1,\n")
		if textured:
			file_out.write(".config.visible = 1,\n")
		else:
			file_out.write(".config.visible = 4,\n")
		file_out.write(".solid_color = 0xFF,\n")
		file_out.write(".model = &" + model_name + ",\n")
		file_out.write("};\n")

		file_out.write("static const Engine_Object " +  name + " = {\n")
		file_out.write(".visual = " + name + "_object,\n")
		if col_file != None:
			file_out.write(".collisions = &" + name + "_collisions,\n")
		else:
			file_out.write(".collisions = NULL,\n")
		file_out.write(".visual_type = ENGINE_VISUAL_MODEL,\n")
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
includes = []
textures = []
models = []

print("Processing Textures...")

for conf_path in glob.glob(os.path.join(glob.escape(args.data_folder), "**/*.json"), recursive=True):
	conf_rel_path = Path(conf_path).relative_to(Path(args.data_folder))
	conf_rel_out_path = Path(conf_rel_path.parent.joinpath(conf_rel_path.stem))
	conf_out_path = Path(os.path.join(args.output_folder, conf_rel_out_path))
	conf_out_path_h = str(conf_out_path) + ".h"
	conf_name = str(conf_rel_out_path).replace('/','_')
	if not conf_out_path.parent.exists() :
		conf_out_path.parent.mkdir(parents=True)
	with open(conf_path, "rt") as conf_file:
		conf = json.load(conf_file)
		if conf.get("texture_path") != None:
			print(conf_path)
			image_path = os.path.join(glob.escape(args.data_folder), conf["texture_path"])
			image_rel_path = Path(image_path).relative_to(Path(args.data_folder))
			image_out_path = str(image_rel_path.parent.joinpath(image_rel_path.stem))
			tex = Image.open(image_path)
			tex = tex.resize((conf["width"], conf["height"]))
			tex = tex.convert("L")
			with open(conf_out_path_h, "tx") as file_out:
				file_out.write("static const L3_COLORTYPE " + conf_name + "_data[" + str(conf["width"] * conf["height"]) + "] = {\n")
				for p in list(tex.getdata()):
					file_out.write(str(p) + ",")
				file_out.write("};\n")
				file_out.write("static const L3_Texture " + conf_name + "= {\n")
				file_out.write(".width = " + str(conf["width"]) + ",\n")
				file_out.write(".height = " + str(conf["height"]) + ",\n")
				file_out.write(".data = " + conf_name + "_data,\n")
				file_out.write("};\n")
			textures.append((conf_name, conf["width"], conf["height"]))
			includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")

print("Processing Models...")

for conf_path in glob.glob(os.path.join(glob.escape(args.data_folder), "**/*.json"), recursive=True):
	conf_rel_path = Path(conf_path).relative_to(Path(args.data_folder))
	conf_rel_out_path = Path(conf_rel_path.parent.joinpath(conf_rel_path.stem))
	conf_out_path = Path(os.path.join(args.output_folder, conf_rel_out_path))
	conf_out_path_h = str(conf_out_path) + ".h"
	conf_name = str(conf_rel_out_path).replace('/','_')
	with open(conf_path, "rt") as conf_file:
		conf = json.load(conf_file)
		if conf.get("model_path") != None:
			print(conf_path)
			model_path = Path(os.path.join(args.data_folder, conf["model_path"]))
			if conf.get("texture"):
				found = False
				for i in textures:
					tex_path = Path(conf["texture"])
					tex_path_stemmed = Path(tex_path.parent.joinpath(tex_path.stem))
					if conf["texture"] == i[0] or str(tex_path_stemmed).replace('/','_') == i[0]:
						generate_model_header(conf_name, model_path, i[0], i[1], i[2], conf_out_path_h)
						includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")
						models.append((conf_name, True))
						found = True
						break
				if not found:
					raise Exception("Texture not found")
			else:
				generate_model_header(conf_name, model_path, None, None, None, conf_out_path_h, True)
				includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")
				models.append((conf_name, False))

print("Processing Objects...")

for conf_path in glob.glob(os.path.join(glob.escape(args.data_folder), "**/*.json"), recursive=True):
	conf_rel_path = Path(conf_path).relative_to(Path(args.data_folder))
	conf_rel_out_path = Path(conf_rel_path.parent.joinpath(conf_rel_path.stem))
	conf_out_path = Path(os.path.join(args.output_folder, conf_rel_out_path))
	conf_out_path_h = str(conf_out_path) + ".h"
	conf_name = str(conf_rel_out_path).replace('/','_') + "_object"
	with open(conf_path, "rt") as conf_file:
		conf = json.load(conf_file)
		if conf.get("model") != None:
			print(conf_path)
			found = False
			for i in models:
				model_path = Path(conf["model"])
				model_path_stemmed = Path(model_path.parent.joinpath(model_path.stem))
				if conf["model"] == i[0] or str(model_path_stemmed).replace('/','_') == i[0]:
					if conf.get("col") != None:
						with open(os.path.join(glob.escape(args.data_folder), conf["col"]), "rt") as col_file:
							generate_object_header(conf_name, i[0], conf_out_path_h, i[1], col_file)
					else:
						generate_object_header(conf_name, i[0], conf_out_path_h, i[1])
					if not ("#include \"" + str(conf_rel_out_path) + ".h\"\n" in includes):
						includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")
					found = True
					break
			if not found:
				raise Exception("Model not found")

			obj_names.append(conf_name)
			visual_names.append(str(conf_rel_out_path).replace('/','_'))
			if conf.get("col") != None:
				collision_names.append(conf_name + "_collisions")
			else:
				collision_names.append("")

print("Processing Billboards...")

for conf_path in glob.glob(os.path.join(glob.escape(args.data_folder), "**/*.json"), recursive=True):
	conf_rel_path = Path(conf_path).relative_to(Path(args.data_folder))
	conf_rel_out_path = Path(conf_rel_path.parent.joinpath(conf_rel_path.stem))
	conf_out_path = Path(os.path.join(args.output_folder, conf_rel_out_path))
	conf_out_path_h = str(conf_out_path) + ".h"
	conf_name = str(conf_rel_out_path).replace('/','_')
	conf_name_b = conf_name + "_b"
	with open(conf_path, "rt") as conf_file:
		conf = json.load(conf_file)
		if conf.get("billboard_path") != None:
			print(conf_path)
			bill_path = os.path.join(glob.escape(args.data_folder), conf["billboard_path"])
			generate_billboard_header(os.path.join(args.tools_path, "texture2billboard.py"), conf_name_b, bill_path, conf_out_path, conf["width"], conf["height"])
			includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")
			generate_billboard(conf_name_b, conf_out_path_h, conf_name)
			obj_names.append(conf_name)
			visual_names.append(conf_name)
			collision_names.append("")

for i in includes:
	index_header.write(i)
	list_header.write(i)

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
