#define EXPORT __attribute__((visibility("default")))

#if DO_EXPORT_CLASS
#define EXPORT_CLASS EXPORT
#else
#define EXPORT_CLASS
#endif
