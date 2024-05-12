for i in build/*.o; do echo $i; nm -gU -C  $i | grep vtable; done
