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
parser.add_argument('--name', '-n', default=None, type=str)
parser.add_argument('--backface', '-bf', default=1, type=int, help="Set backface culling (0,1,2)")

args = parser.parse_args()

object_in = pywavefront.Wavefront(args.filename_in, collect_faces=True)
file_out = open(args.filename_out, "xt")

data_name = Path(args.filename_in).stem
if args.name is not None:
	data_name = args.name
data_vertices_name = data_name + "_vertices"
#data_polys_name = data_name + "_polys"
data_index_name = data_name + "_indexes"
data_model_name = data_name + "_model"
data_normals_name = data_name + "_normals"

file_out.write("#pragma once\nstatic const L3_Unit " +  data_vertices_name + "[] = {\n")

for count, vertex in enumerate(object_in.vertices):
	x = int(vertex[0] * args.scale)
	y = int(vertex[1] * args.scale)
	z = int(vertex[2] * args.scale)
	file_out.write(str(x) + "," + str(y) + "," + str(z) + ",\n")

file_out.write("};\n")

file_out.write("static const L3_Index " +  data_index_name + "[] = {\n")

total_polys = 0

for mesh in object_in.mesh_list:
	total_polys = total_polys + len(mesh.faces)
	for face in mesh.faces:
		file_out.write(str(face[0]) + ", " + str(face[1]) + ", " + str(face[2]) + ",\n")

file_out.write("};\n")

has_normals = True

file_out.write("static const L3_Unit " +  data_normals_name + "[] = {\n")
for mesh in object_in.mesh_list:
	for faceid, face in enumerate(mesh.faces):
		face_data_len = 6
		normal_offset = 0
		if mesh.materials[0].vertex_format == "T2F_N3F_V3F":
			face_data_len = 8
			normal_offset = 2
		elif mesh.materials[0].vertex_format == "N3F_V3F":
			face_data_len = 6
			normal_offset = 0
		else:
			has_normals = False
			break
			break
		normalx = mesh.materials[0].vertices[faceid * face_data_len * 3 + normal_offset]
		normaly = mesh.materials[0].vertices[faceid * face_data_len * 3 + normal_offset + 1]
		normalz = mesh.materials[0].vertices[faceid * face_data_len * 3 + normal_offset + 2]
		file_out.write(str(normalx) + " * L3_F," + str(normaly) + " * L3_F," + str(normalz) + " * L3_F,\n")
file_out.write("};\n")

file_out.write("static const L3_Model3D " +  data_model_name + " = {\n")
file_out.write(".vertices = " + data_vertices_name + ",\n")
file_out.write(".triangleCount = " + str(int(total_polys)) + ",\n")
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
file_out.write(".config.visible = L3_VISIBLE_MODEL_WIREFRAME,\n")
file_out.write(".solid_color = 0xFF,\n")
file_out.write(".model = &" + data_model_name + ",\n")
file_out.write("};\n")
