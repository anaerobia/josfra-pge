INCDIR = -I$(HDFINC) -I$(HDFEOS_INC) -I$(SZIPINC) -I$(HDFEOS_HOME)/gctp/include
LIBDIR = -L$(HDFLIB) -L$(HDFEOS_LIB) -lhdfeos -lGctp -lmfhdf -ldf -ljpeg -lz $(SZIPLIB)/libsz.a -lm
LIBDIR_CYGWIN = -L$(HDFLIB) -L$(HDFEOS_LIB) -lhdfeos -lGctp -lmfhdf -ldf -ljpeg -lz $(SZIPLIB)/libsz.a -lm /usr/lib/librpc.a
LIBDIR_SUN11 = -L$(HDFLIB) -L$(HDFEOS_LIB) -lhdfeos -lGctp -lmfhdf -ldf -ljpeg -lz $(SZIPLIB)/libsz.a -lm -lnsl
default all:
	@echo " "; echo " "; \
	if [ "$(HDFINC)" = "" ] || [ "$(HDFEOS_INC)" = "" ] || [ "$(SZIPINC)" = "" ] || [ "$(HDFLIB)" = "" ] || [ "$(HDFEOS_LIB)" = "" ] || [ "$(SZIPLIB)" = "" ] ; then \
		echo " --- ERROR: One or more of the environment variables HDFINC,"; \
		echo " --- HDF5INC, HDFEOS5_INC, SZIPINC, HDFLIB, HDF5LIB, HDFEOS5_LIB,"; \
		echo " --- SZIPLIB has not been set. Failed building utility executable."; \
	else \
		echo " ---- Making executable for GDconvert_ij2ll grid convertor ----"; \
		echo "$(CC) $(CFLAGS) -o GDconvert_ij2ll.o $(INCDIR) -c GDconvert_ij2ll.c"; \
		$(CC) $(CFLAGS) -o GDconvert_ij2ll.o $(INCDIR) -c GDconvert_ij2ll.c; \
		if [ $(BRAND) = "cygwin" ] ; then \
			echo "$(CC) $(CFLAGS) -o $(HDFEOS_BIN)/GDconvert_ij2ll GDconvert_ij2ll.o $(LIBDIR_CYGWIN) $(CEXTRAL)"; \
			$(CC) $(CFLAGS) -o $(HDFEOS_BIN)/GDconvert_ij2ll GDconvert_ij2ll.o $(LIBDIR_CYGWIN) $(CEXTRAL); \
		elif [ $(BRAND) = "sun5.11" ] ; then \
			echo "$(CC) $(CFLAGS) -o $(HDFEOS_BIN)/GDconvert_ij2ll GDconvert_ij2ll.o $(LIBDIR_CYGWIN) $(CEXTRAL)"; \
			$(CC) $(CFLAGS) -o $(HDFEOS_BIN)/GDconvert_ij2ll GDconvert_ij2ll.o $(LIBDIR_SUN11) $(CEXTRAL); \
		else \
			echo "$(CC) $(CFLAGS) -o $(HDFEOS_BIN)/GDconvert_ij2ll GDconvert_ij2ll.o $(LIBDIR) $(CEXTRAL)"; \
			$(CC) $(CFLAGS) -o $(HDFEOS_BIN)/GDconvert_ij2ll GDconvert_ij2ll.o $(LIBDIR) $(CEXTRAL); \
		fi; \
		$(RM) $(RMFLAGS) *.o; \
	fi; \
	echo " ";

