import argparse
import sys
import os
import pprint
import array
import struct

def flatten(iterable):
    res = []
    def loop(array):
        for val in array:
            if isinstance(val, list):
                loop(val)
            else:
                res.append(val)
    loop(iterable)
    return res


def index_data(index):
    parts = index.split('/')
    return int(parts[0]) - 1, int(parts[1]) - 1, int(parts[2]) - 1


def dump_buffers(data_buffer, index_buffer, aobj):
    fvertices = flatten(data_buffer)
    findices = flatten(index_buffer)

    # one float is 4 bytes
    vert_bytes_count = len(fvertices) * 4
    ind_bytes_count = len(findices) * 4

    vert_bytes = array.array('f', fvertices)
    ind_bytes = array.array('L', findices)

    with open(aobj, 'wb') as fd:

        # one uint8 to indicate the type of object
        fd.write(struct.pack('c', 1))

        # two uint32 -- verts and inds size in bytes
        fd.write(struct.pack('II', vert_bytes, ind_bytes))

        # then comes the data
        fd.write(vert_bytes)
        fd.write(ind_bytes)
    
    


def process_obj(obj):
    with open(obj, 'r') as fd:
        data = fd.read().splitlines()

    verts = []
    norms = []
    uvs = []

    data_buffer = []
    index_buffer = []

    index_dict = {}
    
    for line in data:
        if line.startswith('#'):
            continue

        if line.startswith('mtllib'):
            # process material here
            continue

        if line.startswith('o'):
            # process start of boject here
            continue

        if line.startswith('v') and not line.startswith('vt') and not line.startswith('vn'):
            parts = line.split(' ')
            verts.append([float(parts[1]), float(parts[2]), float(parts[3])])

        if line.startswith('vn'):
            parts = line.split(' ')
            norms.append([float(parts[1]), float(parts[2]), float(parts[3])])

        if line.startswith('vt'):
            parts = line.split(' ')
            uvs.append([float(parts[1]), float(parts[2])])

        if line.startswith('f'):
            parts = line.split(' ')
            v1, t1, n1 = index_data(parts[1])
            v2, t2, n2 = index_data(parts[2])
            v3, t3, n3 = index_data(parts[3])


            i = len(data_buffer)

            data_buffer.append([verts[v1], uvs[t1], norms[n1]])
            data_buffer.append([verts[v2], uvs[t2], norms[n2]])
            data_buffer.append([verts[v3], uvs[t3], norms[n3]])


            if parts[0] not in index_dict.keys():
                index_dict[parts[0]] = i

            if parts[1] not in index_dict.keys():
                index_dict[parts[1]] = i + 1

            if parts[2] not in index_dict.keys():
                index_dict[parts[2]] = i + 2

                
            index_buffer.append(index_dict[parts[0]])
            index_buffer.append(index_dict[parts[1]])
            index_buffer.append(index_dict[parts[2]])

    dump_buffers(data_buffer, index_buffer, "tree.aobj")



def main():
    parser = argparse.ArgumentParser(prog='mesh-buildre', description='Builds meshes!')

    parser.add_argument(
	'--input-dir',
	'-i',
	action='store',
	dest='input_dir',
	default='./resources/models/',
	help='Print veriosn inormation',
    )

    args, _ = parser.parse_known_args()

    input_dir = args.input_dir

    files = [os.path.join(input_dir, f) for f in os.listdir(input_dir) if os.path.isfile(os.path.join(input_dir, f))]

    for obj_file in files:
        if obj_file.endswith('obj'):
            process_obj(obj_file)



if __name__ == '__main__':
    main()
