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

args = parser.parse_args()

object_in = pywavefront.Wavefront(args.filename_in, collect_faces=True)
file_out = open(args.filename_out, "xt")

data_name = Path(args.filename_in).stem
data_vertices_name = data_name + "_vertices"
#data_polys_name = data_name + "_polys"
data_index_name = data_name + "_indexes"

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
file_out.write("};\n")
