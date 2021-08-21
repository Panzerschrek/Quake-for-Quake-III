dnl ==================================================================
dnl Find out what to build
dnl ==================================================================

QF_WITH_TARGETS(
	tools,
	[  --with-tools=<list>     compile qwalk tools:],
	[modelconv,viewer],dummy
)

if test "x$HAVE_SDL" = xyes; then
	if test "x$ENABLE_clients_sdl" = xyes; then
		QW_TARGETS="$QW_TARGETS qw-client-sdl\$(EXEEXT)"
		NQ_TARGETS="$NQ_TARGETS nq-sdl\$(EXEEXT)"
		QW_DESKTOP_DATA="$QW_DESKTOP_DATA quakeforge-qw-sdl.desktop"
		NQ_DESKTOP_DATA="$NQ_DESKTOP_DATA quakeforge-nq-sdl.desktop"
		CL_TARGETS="$CL_TARGETS SDL"
		VID_TARGETS="$VID_TARGETS libQFsdl.la"
		BUILD_SW=yes
		QF_NEED(vid, [common sdl sw])
		QF_NEED(qw, [client common sdl])
		QF_NEED(nq, [client common sdl])
		QF_NEED(console, [client])
	fi
	CAN_BUILD_SW=yes
	if test "x$ENABLE_clients_sdl32" = xyes; then
		QW_TARGETS="$QW_TARGETS qw-client-sdl32\$(EXEEXT)"
		NQ_TARGETS="$NQ_TARGETS nq-sdl32\$(EXEEXT)"
		QW_DESKTOP_DATA="$QW_DESKTOP_DATA quakeforge-qw-sdl32.desktop"
		NQ_DESKTOP_DATA="$NQ_DESKTOP_DATA quakeforge-nq-sdl32.desktop"
		CL_TARGETS="$CL_TARGETS SDL32"
		VID_TARGETS="$VID_TARGETS libQFsdl32.la"
		BUILD_SW32=yes
		QF_NEED(vid, [common sdl sw32])
		QF_NEED(qw, [client common sdl])
		QF_NEED(nq, [client common sdl])
		QF_NEED(console, [client])
	fi
	CAN_BUILD_SW32=yes
	if test "x$ENABLE_clients_sgl" = xyes; then
		QW_TARGETS="$QW_TARGETS qw-client-sgl\$(EXEEXT)"
		NQ_TARGETS="$NQ_TARGETS nq-sgl\$(EXEEXT)"
		QW_DESKTOP_DATA="$QW_DESKTOP_DATA quakeforge-qw-sgl.desktop"
		NQ_DESKTOP_DATA="$NQ_DESKTOP_DATA quakeforge-nq-sgl.desktop"
		CL_TARGETS="$CL_TARGETS SDL-GL"
		VID_TARGETS="$VID_TARGETS libQFsgl.la"
		BUILD_GL=yes
		CAN_BUILD_GL=yes
		QF_NEED(vid, [common sdl gl])
		QF_NEED(qw, [client common sdl])
		QF_NEED(nq, [client common sdl])
		QF_NEED(console, [client])
	fi
fi

unset TOOLS_TARGETS
if test "x$ENABLE_tools_modelconv" = xyes; then
	TOOLS_TARGETS="$TOOLS_TARGETS modelconv"
fi
if test "x$ENABLE_tools_viewer" = xyes; then
	TOOLS_TARGETS="$TOOLS_TARGETS viewer"
fi

AM_CONDITIONAL(BUILD_MODELCONV, test "$ENABLE_tools_modelconv" = "yes")
AM_CONDITIONAL(BUILD_VIEWER, test "$ENABLE_tools_viewer" = "yes")

QF_PROCESS_NEED(qwalk, [modelconv viewer], a)
