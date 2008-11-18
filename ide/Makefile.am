## Process this file with automake to produce Makefile.in

bin_PROGRAMS = zoinks

zoinks_SOURCES = 							\
		GNU_diff_analyze.cpp				\
		GNU_diff_analyze.h					\
		IDECommon.h							\
		IDEMain.cpp							\
		TDiffDialogs.cpp					\
		TDiffDialogs.h						\
		TDiffListView.cpp					\
		TDiffListView.h						\
		TDiffTextView.cpp					\
		TDiffTextView.h						\
		TDirectoryDiffDocument.cpp			\
		TDirectoryDiffDocument.h			\
		TDirectoryDiffListView.cpp			\
		TDirectoryDiffListView.h			\
		TDirectoryDiffNode.cpp				\
		TDirectoryDiffNode.h				\
		TEditorTextView.cpp					\
		TEditorTextView.h					\
		TFileDiffDocument.cpp				\
		TFileDiffDocument.h					\
		TFilePathBehavior.cpp				\
		TFilePathBehavior.h					\
		TFunctionScanner.cpp				\
		TFunctionScanner.h					\
		TFunctionsMenu.cpp					\
		TFunctionsMenu.h					\
		THTMLBehavior.cpp					\
		THTMLBehavior.h						\
		THTMLCommands.h						\
		TIDEApplication.cpp					\
		TIDEApplication.h					\
		TLanguage.cpp						\
		TLanguage.h							\
		TLineNumberBehavior.cpp				\
		TLineNumberBehavior.h				\
		TLogDocument.cpp					\
		TLogDocument.h						\
		TLogDocumentOwner.cpp				\
		TLogDocumentOwner.h					\
		TLogViewBehavior.cpp				\
		TLogViewBehavior.h					\
		TProjectCommands.h					\
		TProjectDocument.cpp				\
		TProjectDocument.h					\
		TSyntaxScanner.cpp					\
		TSyntaxScanner.h					\
		TSyntaxTextView.cpp					\
		TSyntaxTextView.h					\
		TTeXBehavior.cpp					\
		TTeXBehavior.h						\
		TTeXCommands.h						\
		TTextDocument.cpp					\
		TTextDocument.h

EXTRA_DIST = 								\
		Pixmaps/LeftArrow.xpm				\
		Pixmaps/LeftArrowDisabled.xpm		\
		Pixmaps/RightArrow.xpm				\
		Pixmaps/RightArrowDisabled.xpm		\
		Pixmaps/zoinks.xpm


zoinks_LDADD = @IDE_LDADD@ @X_LIBRARY_PATH@ @INTLLIBS@

zoinks_DEPENDENCIES = @IDE_LDADD@

AM_CXXFLAGS = @CXXFLAGS@ @MY_CXXFLAGS@ -I$(top_srcdir) -DLOCALEDIR=\""$(localedir)"\"
