import pywavefront
import argparse
from pathlib import Path
import os
import glob
import json
from PIL import Image

MODEL_SCALE = 512
MUL_UV = 1

class FACE:
	def __init__(self, vertice_indexes: tuple, uv: tuple, normals: tuple, material_index: int):
		self.vertice_indexes = vertice_indexes
		self.uv = uv
		self.material_index = material_index
		self.normals = normals

class OBJ:
	def __init__(self, object_in):
		self.vertices = []
		self.faces = []
		for vertex in object_in.vertices:
			self.vertices.append((vertex[0], vertex[1], vertex[2]))

		for mesh in object_in.mesh_list:
			face_total = 0
			for matid, mat in enumerate(mesh.materials):
				face_data_len = 5
				if mat.vertex_format == "T2F_N3F_V3F":
					face_data_len = 8
				elif mat.vertex_format == "T2F_V3F":
					face_data_len = 5
				elif mat.vertex_format == "N3F_V3F":
					face_data_len = 6
				elif mat.vertex_format == "V3F":
					face_data_len = 3
				else:
					raise Exception("Unsupported face format")
				face_cnt = len(mat.vertices) / (face_data_len * 3)
				if float(int(face_cnt)) != face_cnt:
					raise Exception("Invalid face count")
				face_cnt = int(face_cnt)
				for fi in range(face_cnt):
					face = mesh.faces[face_total + fi]
					face0uv = None
					face1uv = None
					face2uv = None
					if face_data_len == 8 or face_data_len == 5:
						face0uv = (mat.vertices[fi * face_data_len * 3], mat.vertices[fi * face_data_len * 3 + 1])
						face1uv = (mat.vertices[fi * face_data_len * 3 + face_data_len], mat.vertices[fi * face_data_len * 3 + face_data_len + 1])
						face2uv = (mat.vertices[fi * face_data_len * 3 + face_data_len * 2], mat.vertices[fi * face_data_len * 3 + face_data_len * 2 + 1])

					face0n = None
					face1n = None
					face2n = None
					if face_data_len == 8:
						face0n = (mat.vertices[fi * face_data_len * 3 + 2], mat.vertices[fi * face_data_len * 3 + 3], mat.vertices[fi * face_data_len * 3 + 4])
						face1n = (mat.vertices[fi * face_data_len * 3 + face_data_len + 2], mat.vertices[fi * face_data_len * 3 + face_data_len + 3], mat.vertices[fi * face_data_len * 3 + face_data_len + 4])
						face2n = (mat.vertices[fi * face_data_len * 3 + face_data_len * 2 + 2], mat.vertices[fi * face_data_len * 3 + face_data_len * 2 + 3], mat.vertices[fi * face_data_len * 3 + face_data_len * 2 + 4])
					self.faces.append(FACE((face[0], face[1], face[2]), (face0uv, face1uv, face2uv), (face0n, face1n, face2n), matid))
				face_total += face_cnt

def generate_model_header_textured(name, model_path, textures, out_path):
	data_vertices_name = name + "_vertices"
	data_index_name = name + "_indexes"
	data_texture_name = name + "_textures"
	data_uv_name = name + "_UVs"
	data_normals_name = name + "_Normals"
	data_texindex_name = name + "_indexes_texture"
	data_model_name = name

	object_in = pywavefront.Wavefront(model_path, collect_faces=True)
	if len(object_in.mesh_list) > 1:
		raise Exception("More than 1 mesh in obj file")

	mesh = object_in.mesh_list[0]

	obj = OBJ(object_in)

	if len(textures) != len(mesh.materials):
		raise Exception("Texture count {} doesn't match object's material count {}".format(len(textures), len(mesh.materials)))

	file_out = open(out_path, "xt")

	file_out.write("#pragma once\nstatic const L3_Unit " +  data_vertices_name + "[] = {\n")
	for vertex in obj.vertices:
		x = int(vertex[0] * MODEL_SCALE)
		y = int(vertex[1] * MODEL_SCALE)
		z = int(vertex[2] * MODEL_SCALE)
		file_out.write(str(x) + "," + str(y) + "," + str(z) + ",\n")
	file_out.write("};\n")

	file_out.write("static const L3_Index " +  data_index_name + "[] = {\n")
	for face in obj.faces:
		file_out.write(str(face.vertice_indexes[0]) + ", " + str(face.vertice_indexes[1]) + ", " + str(face.vertice_indexes[2]) + ",\n")
	file_out.write("};\n")

	file_out.write("static const L3_Texture *" +  data_texture_name + "[] = {\n")
	for tex in textures:
		file_out.write("&" + tex[0] + ",\n")
	file_out.write("};\n")

	file_out.write("static const L3_Index " +  data_texindex_name + "[] = {\n")
	for face in obj.faces:
		if face.material_index is None:
			file_out.write(str(-1) + ",")
		else:
			file_out.write(str(face.material_index) + ",")
	file_out.write("};\n")

	file_out.write("static const L3_Unit " +  data_uv_name + "[] = {\n")
	for face in obj.faces:
		if face.material_index is None:
			file_out.write(str(0) + ", " + str(0) + ",\n")
			file_out.write(str(0) + ", " + str(0) + ",\n")
			file_out.write(str(0) + ", " + str(0) + ",\n")
		else:
			texture_width = textures[face.material_index][1]
			texture_height = textures[face.material_index][2]
			# with fix for UV being the wrong way for some reason
			file_out.write(str(int(face.uv[0][0] * texture_width * MUL_UV)) + ", " + str(int(texture_height * MUL_UV - face.uv[0][1] * texture_height * MUL_UV)) + ",\n")
			file_out.write(str(int(face.uv[1][0] * texture_width * MUL_UV)) + ", " + str(int(texture_height * MUL_UV - face.uv[1][1] * texture_height * MUL_UV)) + ",\n")
			file_out.write(str(int(face.uv[2][0] * texture_width * MUL_UV)) + ", " + str(int(texture_height * MUL_UV - face.uv[2][1] * texture_height * MUL_UV)) + ",\n")
	file_out.write("};\n")

	has_normals = True

	if face.normals[0] is None:
		has_normals = False
	else:
		file_out.write("static const L3_Unit " +  data_normals_name + "[] = {\n")
		for face in obj.faces:
			file_out.write(str(face.normals[0][0]) + " * L3_F," + str(face.normals[0][1]) + " * L3_F," + str(face.normals[0][2]) + " * L3_F,\n")
		file_out.write("};\n")

	file_out.write("static const L3_Model3D " +  data_model_name + " = {\n")
	file_out.write(".vertices = " + data_vertices_name + ",\n")
	file_out.write(".triangleCount = " + str(len(mesh.faces)) + ",\n")
	file_out.write(".vertexCount = " + str(len(object_in.vertices)) + ",\n")
	file_out.write(".triangles = " + data_index_name + ",\n")
	file_out.write(".triangleTextures = " + data_texture_name + ",\n")
	file_out.write(".triangleUVs = " + data_uv_name + ",\n")
	file_out.write(".triangleTextureIndex = " + data_texindex_name + ",\n")
	if has_normals:
		file_out.write(".triangleNormals = " + data_normals_name + ",\n")
	else:
		file_out.write(".triangleNormals = NULL,\n")
	file_out.write("};\n")
	file_out.close()

def generate_model_header_notex(name, model_path, out_path):
	data_vertices_name = name + "_vertices"
	data_index_name = name + "_indexes"
	data_normals_name = name + "_Normals"
	data_model_name = name

	object_in = pywavefront.Wavefront(model_path, collect_faces=True)

	if len(object_in.mesh_list) > 1:
		raise Exception("More than 1 mesh in obj file")

	mesh = object_in.mesh_list[0]

	obj = OBJ(object_in)

	file_out = open(out_path, "xt")

	file_out.write("#pragma once\nstatic const L3_Unit " +  data_vertices_name + "[] = {\n")
	for vertex in obj.vertices:
		x = int(vertex[0] * MODEL_SCALE)
		y = int(vertex[1] * MODEL_SCALE)
		z = int(vertex[2] * MODEL_SCALE)
		file_out.write(str(x) + "," + str(y) + "," + str(z) + ",\n")
	file_out.write("};\n")

	file_out.write("static const L3_Index " +  data_index_name + "[] = {\n")
	for face in obj.faces:
		file_out.write(str(face.vertice_indexes[0]) + ", " + str(face.vertice_indexes[1]) + ", " + str(face.vertice_indexes[2]) + ",\n")
	file_out.write("};\n")

	has_normals = True

	if face.normals[0] is None:
		has_normals = False
	else:
		file_out.write("static const L3_Unit " +  data_normals_name + "[] = {\n")
		for face in obj.faces:
			file_out.write(str(face.normals[0][0]) + " * L3_F," + str(face.normals[0][1]) + " * L3_F," + str(face.normals[0][2]) + " * L3_F,\n")
		file_out.write("};\n")

	file_out.write("static const L3_Model3D " +  data_model_name + " = {\n")
	file_out.write(".vertices = " + data_vertices_name + ",\n")
	file_out.write(".triangleCount = " + str(len(mesh.faces)) + ",\n")
	file_out.write(".vertexCount = " + str(len(object_in.vertices)) + ",\n")
	file_out.write(".triangles = " + data_index_name + ",\n")
	file_out.write(".triangleTextures = NULL,\n")
	file_out.write(".triangleUVs = NULL,\n")
	file_out.write(".triangleTextureIndex = NULL,\n")
	if has_normals:
		file_out.write(".triangleNormals = " + data_normals_name + ",\n")
	else:
		file_out.write(".triangleNormals = NULL,\n")
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
			file_out.write(".config.visible = L3_VISIBLE_MODEL_TEXTURED,\n")
		else:
			file_out.write(".config.visible = L3_VISIBLE_MODEL_SOLID,\n")
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
						generate_model_header_textured(conf_name, model_path, [i], conf_out_path_h)
						includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")
						models.append((conf_name, True))
						found = True
						break
				if not found:
					raise Exception("Texture {} not found".format(conf["texture"]))
			elif conf.get("textures"):
				local_model_textures = []
				for model_tex in conf["textures"]:
					found = False
					for tex in textures:
						tex_path = Path(model_tex)
						tex_path_stemmed = Path(tex_path.parent.joinpath(tex_path.stem))
						if model_tex == tex[0] or str(tex_path_stemmed).replace('/','_') == tex[0]:
							local_model_textures.append(tex)
							found = True
							break
					if not found:
						raise Exception("Texture {} not found".format(model_tex))
				generate_model_header_textured(conf_name, model_path, local_model_textures, conf_out_path_h)
				includes.append("#include \"" + str(conf_rel_out_path) + ".h\"\n")
				models.append((conf_name, True))
			else:
				generate_model_header_notex(conf_name, model_path, conf_out_path_h)
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
