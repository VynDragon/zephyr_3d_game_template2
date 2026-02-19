import pywavefront
import argparse
from pathlib import Path
from PIL import Image

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

parser = argparse.ArgumentParser(
                    prog='obj2c',
                    description='convert obj file to PL3D-KC object structure')

parser.add_argument('filename_in')
parser.add_argument('filename_out')
parser.add_argument('--scale', '-s', default=512, type=int)
parser.add_argument('--name', '-n', default=None, type=str)
parser.add_argument('--backface', '-bf', default=1, type=int, help="Set backface culling (0,1,2)")
parser.add_argument('--texture_width', '-tw', default=32, type=int, help="Texture width")
parser.add_argument('--texture_height', '-th', default=32, type=int, help="Texture height")
parser.add_argument('--texture_name', '-tn', default=None, type=str, help="Override Textures names")

args = parser.parse_args()

object_in = pywavefront.Wavefront(args.filename_in, collect_faces=True)
file_out = open(args.filename_out, "xt")

data_name = Path(args.filename_in).stem
if args.name is not None:
	data_name = args.name
data_vertices_name = data_name + "_vertices"
#data_polys_name = data_name + "_polys"
data_index_name = data_name + "_indexes"
data_texture_name = data_name + "_textures"
data_texture_name_width = data_name + "_textures_width"
data_texture_name_height = data_name + "_textures_height"
data_uv_name = data_name + "_UVs"
data_texindex_name = data_name + "texindex"
data_model_name = data_name + "_model"

if len(object_in.mesh_list) > 1:
	raise Exception("More than 1 mesh in obj file")

mesh = object_in.mesh_list[0]

textures = {}
for name, mat in enumerate(mesh.materials):
	if mat.texture:
		tex = Image.open(mat.texture.find())
		tex = tex.resize((args.texture_width, args.texture_height))
		tex = tex.convert("L")
		file_out.write("static const L3_COLORTYPE " +  data_name + name + "texturedata[" + str(args.texture_width * args.texture_height) + "] = {\n")
		for p in list(tex.getdata()):
			file_out.write(str(p) + ",")
		file_out.write("};\n")
		textures[data_name + name + "texturedata"] = index

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
for name, i in textures.items():
	file_out.write("&" + name + ",\n")
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
		texture_width = args.texture_width
		texture_height = args.textures_height
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
file_out.write(".config.backfaceCulling = " + str(args.backface) + ",\n")
file_out.write(".config.visible = L3_VISIBLE_MODEL_TEXTURED,\n")
file_out.write(".solid_color = 0xFF,\n")
file_out.write(".model = &" + data_model_name + ",\n")
file_out.write("};\n")
