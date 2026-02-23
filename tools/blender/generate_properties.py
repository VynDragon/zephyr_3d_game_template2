import bpy

for obj in bpy.data.objects:
	if obj.type == 'MESH':
		textured = False
		color = None
		is_textured = False
		if hasattr(obj, "material_slots"):
			for mat in obj.material_slots:
				node_tree = mat.material.node_tree
				for node in node_tree.nodes:
					if hasattr(node, "image"):
						is_textured = True
					if 'Base Color' in node.inputs and color is None:
						color = node.inputs['Base Color'].default_value
		if not "visual_type" in obj:
			obj["visual_type"] = 2
		if not "view_range" in obj:
			obj["view_range"] = 131072
		if not "backfaceCulling" in obj:
			obj["backfaceCulling"] = 1
		if not "visible" in obj:
			if is_textured:
				obj["visible"] = 2
			else:
				obj["visible"] = 8
		if not "solid_color" in obj:
			obj["solid_color"] = 128
			if not (color is None):
				obj["solid_color"] = int(((color[0] + color[1] + color[2]) / 3.0) * 255)
			elif obj.active_material:
				obj["solid_color"] = int(((obj.active_material.diffuse_color[0] + obj.active_material.diffuse_color[1] + obj.active_material.diffuse_color[2]) / 3.0) * 255)
		if not "visual_i" in obj:
			obj["visual_i"] = obj.name.split('.')[0]
		if not "collisions" in obj:
			obj["collisions"] = ""
