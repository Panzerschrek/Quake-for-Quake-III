AUTOMAKE_OPTIONS=   foreign
ACLOCAL_AMFLAGS = -I m4

noinst_LIBRARIES= libqwalk.a
bin_PROGRAMS= modelconv viewer
EXTRA_PROGRAMS=modelconv viewer

libqwalk_a_SOURCES= anorms.c image.c image_jpeg.c image_pcx.c image_tga.c \
                    matrix.c model.c model_md2.c model_md3.c model_mdl.c \
                    model_mdo.c palettes.c util.c

modelconv_SOURCES=modelconv.c
modelconv_LDADD=libqwalk.a $(LIBS)
modelconv_LDFLAGS=$(LDFLAGS)
modelconv_DEPENDENCIES=libqwalk.a

viewer_SOURCES=viewer.c v_font.c
viewer_LDADD=libqwalk.a $(SDL_LIBS) $(GL_LIBS) $(LIBS)
viewer_LDFLAGS=$(LDFLAGS)
viewer_CFLAGS=$(SDL_CFLAGS)
viewer_DEPENDENCIES=libqwalk.a
