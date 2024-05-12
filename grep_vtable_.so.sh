for i in build/*.so; do echo $i; nm -gU -C  $i | grep vtable; done
