kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kbounce/sounds FILES death.au reflect.au wallstart.au wallend.au seconds.au timeout.au )

kde4_footer()



#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = foreign
#
#sound_DATA = death.au reflect.au wallstart.au wallend.au seconds.au timeout.au
#sounddir = $(kde_datadir)/kbounce/sounds
# 
#EXTRA_DIST = $(sound_DATA)
#
