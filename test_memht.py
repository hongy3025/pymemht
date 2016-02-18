import os
import mmap
import memht

def create_mmap_file(filename, size):
    with open(filename, 'wb') as f:
        f.seek(size-1)
        f.write(b'\x00')

def memory_map(filename, access=mmap.ACCESS_WRITE):
    size = os.path.getsize(filename)
    fd = os.open(filename, os.O_RDWR)
    return mmap.mmap(fd, size, access=access)

def init():
    create_mmap_file('datafile', 100*1024*1024)
    m = memory_map('datafile')
    ht = memht.MemoryHashTable(m, 12, 4, 1000000)
    print 'create', ht
    ht['hongy'] = 'aaaa'
    for x in xrange(10):
        ht[str(x)] = str(x)
    value = ht['hongy']
    print 'value of hongy', value
    print 'size of data tbl', len(ht)
    for k, v in ht.iteritems():
        print 'iter', k, v

init()
