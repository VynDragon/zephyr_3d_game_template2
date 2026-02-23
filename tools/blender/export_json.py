import bpy
import json
from bpy_extras.io_utils import ExportHelper
from bpy.types import Operator
from bpy.props import StringProperty
from math import pi

MUL = 512

def write_json(context, filepath):
	print("running write_json...")
	json_data = []
	for obj in bpy.data.objects:
		if obj.type == 'MESH' and obj.hide_viewport == False:
			obj_json_transform= {
				"translation": {"x": obj.location.x * MUL, "y": obj.location.y * MUL, "z": obj.location.z * MUL, "w": MUL},
				"rotation": {"x": obj.rotation_euler.x / (2 * pi) * MUL, "y": obj.rotation_euler.y / (2 * pi) * MUL, "z": obj.rotation_euler.z / (2 * pi) * MUL, "w": MUL},
				"scale": {"x": obj.scale.x * MUL, "y": obj.scale.y * MUL, "z": obj.scale.z * MUL, "w": 0},
				}
			obj_json = {
					"visual_type": obj["visual_type"],
					"view_range": obj["view_range"],
					"backfaceCulling": obj["backfaceCulling"],
					"visible": obj["visible"],
					"solid_color": obj["solid_color"],
					"transform": obj_json_transform,
					"visual_i": obj["visual_i"],
					"collisions": obj["collisions"],
				}
			json_data.append(obj_json)

	f = open(filepath, 'w', encoding='utf-8')
	f.write(json.dumps(json_data, indent=4))
	f.close()

	return {'FINISHED'}



class ExportL3JSON(Operator, ExportHelper):
	bl_idname = "export_l3.json"
	bl_label = "Export L3 JSON"

	filename_ext = ".json"

	filter_glob: StringProperty(
		default="*.json",
		options={'HIDDEN'},
		maxlen=255,  # Max internal buffer length, longer would be clamped.
	)

	def execute(self, context):
		print(self)
		return write_json(context, self.filepath)


# Only needed if you want to add into a dynamic menu
def menu_func_import(self, context):
	self.layout.operator(ExportL3JSON.bl_idname, text="L3 (.json)")

def register():
	bpy.utils.register_class(ExportL3JSON)


def unregister():
	bpy.utils.unregister_class(ExportL3JSON)

if __name__ == "__main__":
	register()

	bpy.ops.export_l3.json('INVOKE_DEFAULT')

	#unregister()
