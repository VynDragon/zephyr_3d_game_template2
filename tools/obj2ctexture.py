import pywavefront
import argparse
from pathlib import Path
from PIL import Image

parser = argparse.ArgumentParser(
                    prog='obj2c',
                    description='convert obj file to PL3D-KC object structure')

parser.add_argument('filename_in')
parser.add_argument('filename_out')
parser.add_argument('--scale', '-s', default=512, type=int)
TEXSQUARE_SIZE = 32

args = parser.parse_args()

object_in = pywavefront.Wavefront(args.filename_in, collect_faces=True)
file_out = open(args.filename_out, "xt")

data_name = Path(args.filename_in).stem
data_vertices_name = data_name + "_vertices"
#data_polys_name = data_name + "_polys"
data_index_name = data_name + "_indexes"
data_texture_name = data_name + "_textures"
data_uv_name = data_name + "_UVs"
data_texindex_name = data_name + "texindex"

file_out.write("#pragma once\nstatic const S3L_Unit " +  data_vertices_name + "[] = {\n")

for count, vertex in enumerate(object_in.vertices):
	x = int(vertex[0] * args.scale)
	y = int(vertex[1] * args.scale)
	z = int(vertex[2] * args.scale)
	file_out.write(str(x) + "," + str(y) + "," + str(z) + ",\n")

file_out.write("};\n")

file_out.write("static const S3L_Index " +  data_index_name + "[] = {\n")

total_polys = 0

for mesh in object_in.mesh_list:
	total_polys = total_polys + len(mesh.faces)
	for face in mesh.faces:
		file_out.write(str(face[0]) + ", " + str(face[1]) + ", " + str(face[2]) + ",\n")

file_out.write("};\n")

textures = {}
index = 0
for name, value in object_in.materials.items():
	if value.texture:
		tex = Image.open(value.texture.find())
		tex = tex.resize((TEXSQUARE_SIZE, TEXSQUARE_SIZE))
		tex = tex.convert("L")
		file_out.write("static const S3L_Unit " +  data_name + name + "texturedata[" + str(TEXSQUARE_SIZE * TEXSQUARE_SIZE) + "] = {\n")
		for p in list(tex.getdata()):
			file_out.write(str(p) + ",")
		file_out.write("};\n")
		textures[data_name + name + "texturedata"] = index
		index = index +1

file_out.write("static const S3L_Unit *" +  data_texture_name + "[] = {\n")
for i, n in textures.items():
	file_out.write(i + ",\n")
file_out.write("};\n")

file_out.write("static const S3L_Unit " +  data_uv_name + "[] = {\n")
for mesh in object_in.mesh_list:
	for faceid, face in enumerate(mesh.faces):
		face0uv = (mesh.materials[0].vertices[faceid * 5 * 3], mesh.materials[0].vertices[faceid * 5 * 3+ 1])
		face1uv = (mesh.materials[0].vertices[faceid * 5 * 3 + 5], mesh.materials[0].vertices[faceid * 5 * 3 + 6])
		face2uv = (mesh.materials[0].vertices[faceid * 5 * 3 + 10], mesh.materials[0].vertices[faceid * 5 * 3 + 11])
		file_out.write(str(int(face0uv[0] * TEXSQUARE_SIZE)) + ", " + str(int(face0uv[1] * TEXSQUARE_SIZE)) + ",\n")
		file_out.write(str(int(face1uv[0] * TEXSQUARE_SIZE)) + ", " + str(int(face1uv[1] * TEXSQUARE_SIZE)) + ",\n")
		file_out.write(str(int(face2uv[0] * TEXSQUARE_SIZE)) + ", " + str(int(face2uv[1] * TEXSQUARE_SIZE)) + ",\n")
file_out.write("};\n")

file_out.write("static const S3L_Index " +  data_texindex_name + "[] = {\n")
for mesh in object_in.mesh_list:
	for faceid, face in enumerate(mesh.faces):
		texname = textures.get(data_name + mesh.materials[0].name + "texturedata")
		if texname is None:
			file_out.write(str(-1) + ",")
		else:
			file_out.write(str(texname) + ",")
file_out.write("};\n")

file_out.write("static const S3L_Model3D " +  data_name + " = {\n")
file_out.write(".vertices = " + data_vertices_name + ",\n")
file_out.write(".triangleCount = " + str(int(total_polys)) + ",\n")
file_out.write(".vertexCount = " + str(len(object_in.vertices)) + ",\n")
file_out.write(".triangles = " + data_index_name + ",\n")
file_out.write(".customTransformMatrix = 0,\n")
file_out.write(".transform.scale.x = S3L_F,\n")
file_out.write(".transform.scale.y = S3L_F,\n")
file_out.write(".transform.scale.z = S3L_F,\n")
file_out.write(".transform.scale.w = 0,\n")
file_out.write(".transform.translation.x = 0,\n")
file_out.write(".transform.translation.y = 0,\n")
file_out.write(".transform.translation.z = 0,\n")
file_out.write(".transform.translation.w = S3L_F,\n")
file_out.write(".transform.rotation.x = 0,\n")
file_out.write(".transform.rotation.y = 0,\n")
file_out.write(".transform.rotation.z = 0,\n")
file_out.write(".transform.rotation.w = S3L_F,\n")
file_out.write(".config.backfaceCulling = 2,\n")
file_out.write(".config.visible = 1,\n")
file_out.write(".triangleTextures = " + data_texture_name + ",\n")
file_out.write(".triangleUVs = " + data_uv_name + ",\n")
file_out.write(".triangleTextureIndex = " + data_texindex_name + ",\n")
file_out.write("};\n")
