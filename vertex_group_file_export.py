import bpy

#see https://blender.stackexchange.com/questions/145972/how-to-select-and-assign-vertices-to-a-vertex-group-via-python
def write_vertex_groups(context, filepath):
    obj = bpy.context.object
    print("running write_some_data...")
    f = open(filepath, 'w', encoding='utf-8')
    vertex_groups = []
    for group_name, group in obj.vertex_groups.items():
        f.write("g %d %s" % (group.index, group_name))
        f.write("\n")
        vertex_groups.append(group)
    for vertex in obj.data.vertices:
        f.write("%d" % vertex.index)
        for vg in vertex.groups:
            f.write(" %d" % vg.group)
        f.write("\n") 
    f.close()

    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportVertexGroups(Operator, ExportHelper):
    """This appears in the tooltip of the operator and in the generated docs"""
    bl_idname = "export.vertex_groups"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export vertex groups"

    # ExportHelper mixin class uses this
    filename_ext = ".txt"

    filter_glob: StringProperty(
        default="*.txt",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.
    use_setting: BoolProperty(
        name="Example Boolean",
        description="Example Tooltip",
        default=True,
    )

    type: EnumProperty(
        name="Example Enum",
        description="Choose between two items",
        items=(
            ('OPT_A', "First Option", "Description one"),
            ('OPT_B', "Second Option", "Description two"),
        ),
        default='OPT_A',
    )

    def execute(self, context):
        return write_vertex_groups(context, self.filepath)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportSomeData.bl_idname, text="Text Export Operator")


# Register and add to the "file selector" menu (required to use F3 search "Text Export Operator" for quick access).
def register():
    bpy.utils.register_class(ExportVertexGroups)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportVertexGroups)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.export.vertex_groups('INVOKE_DEFAULT')
