import named_sema

sema = named_sema.NamedSemaphore()
sema.create('my_sema', 0, 1)

print 'semaphore created'
sema.wait()
print 'semaphore wait over'

