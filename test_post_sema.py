import named_sema

sema = named_sema.NamedSemaphore()
result = sema.open('my_sema')
print 'open semaphore', result
sema.post()

