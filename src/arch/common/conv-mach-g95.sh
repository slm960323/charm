# GNU f95

CMK_CF90=`which f95 2>/dev/null`
CMK_FPP="/lib/cpp -P -CC"
CMK_CF90="$CMK_CF90 -fpic"
CMK_CF90_FIXED="$CMK_CF90 -ffixed-form "
CMK_F90LIBS="-lgfortran "
CMK_MOD_NAME_ALLCAPS=
CMK_MOD_EXT="mod"
CMK_F90_USE_MODDIR=1
CMK_F90_MODINC="-I"

