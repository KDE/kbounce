



########### install files ###############

install( FILES death.au reflect.au wallstart.au wallend.au seconds.au timeout.au  DESTINATION  ${DATA_INSTALL_DIR}/kbounce/sounds )




#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = foreign
#
#sound_DATA = death.au reflect.au wallstart.au wallend.au seconds.au timeout.au
#sounddir = $(kde_datadir)/kbounce/sounds
# 
#EXTRA_DIST = $(sound_DATA)
#
