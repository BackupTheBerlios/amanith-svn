#ifdef _FONTS_TRUETYPE_MODULE
	FT_USE_MODULE(tt_driver_class)
#endif

#ifdef _FONTS_TYPE1_MODULE
	FT_USE_MODULE(t1_driver_class)
#endif

#ifdef _FONTS_CFF_MODULE
	FT_USE_MODULE(cff_driver_class)
#endif

#ifdef _FONTS_CID_MODULE
	FT_USE_MODULE(t1cid_driver_class)
#endif

FT_USE_MODULE(psaux_module_class)
FT_USE_MODULE(psnames_module_class)
FT_USE_MODULE(pshinter_module_class)
FT_USE_MODULE(sfnt_module_class)

#ifdef _FONTS_TYPE42_MODULE
	FT_USE_MODULE(t42_driver_class)
#endif

#ifdef _FONTS_PFR_MODULE
	FT_USE_MODULE(pfr_driver_class)
#endif
