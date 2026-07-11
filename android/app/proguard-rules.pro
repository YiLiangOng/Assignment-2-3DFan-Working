# Add project specific ProGuard rules here.
# By default, the flags in this file are applied to the debug build as well.

# Keep native methods
-keepclasseswithmembernames class * {
    native <methods>;
}
