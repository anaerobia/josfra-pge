INCDIR = -I$(HDFINC) -I$(HDFEOS_INC)
LIBDIR = -L$(HDFLIB) -L$(HDFEOS_LIB) -lhdfeos -lGctp -lmfhdf -ldf -ljpeg -lz $(SZIPLIB)/libsz.a -lm
# /usr/lib/librpc.a

default all:
	@echo " "; echo " "; \
	echo " ---- Making *.c samples for swath ----"; \
	$(CC) $(CFLAGS) -o SetupSwath.o $(INCDIR) -c SetupSwath.c; \
	$(CC) $(CFLAGS) -o SetupSwath SetupSwath.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o DefineFields.o $(INCDIR) -c DefineFields.c; \
	$(CC) $(CFLAGS) -o DefineFields DefineFields.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o WriteFields.o $(INCDIR) -c WriteFields.c; \
	$(CC) $(CFLAGS) -o WriteFields WriteFields.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o AppendField.o $(INCDIR) -c AppendField.c; \
	$(CC) $(CFLAGS) -o AppendField AppendField.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o InquireSwath.o $(INCDIR) -c InquireSwath.c; \
	$(CC) $(CFLAGS) -o InquireSwath InquireSwath.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o ReadFields.o $(INCDIR) -c ReadFields.c; \
	$(CC) $(CFLAGS) -o ReadFields ReadFields.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o SubsetSwath.o $(INCDIR) -c SubsetSwath.c; \
	$(CC) $(CFLAGS) -o SubsetSwath SubsetSwath.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o WriteDimscaleSwath.o $(INCDIR) -c WriteDimscaleSwath.c; \
	$(CC) $(CFLAGS) -o WriteDimscaleSwath WriteDimscaleSwath.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o ReadDimscaleSwath.o $(INCDIR) -c ReadDimscaleSwath.c; \
	$(CC) $(CFLAGS) -o ReadDimscaleSwath ReadDimscaleSwath.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	echo " "; echo " "; \
	echo " ---- Making *.c samples for point ----"; \
	$(CC) $(CFLAGS) -o SetupPoint.o $(INCDIR) -c SetupPoint.c; \
	$(CC) $(CFLAGS) -o SetupPoint SetupPoint.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o DefineLevels.o $(INCDIR) -c DefineLevels.c; \
	$(CC) $(CFLAGS) -o DefineLevels DefineLevels.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o WriteLevels.o $(INCDIR) -c WriteLevels.c; \
	$(CC) $(CFLAGS) -o WriteLevels WriteLevels.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o UpdateLevels.o $(INCDIR) -c UpdateLevels.c; \
	$(CC) $(CFLAGS) -o UpdateLevels UpdateLevels.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o ReadLevels.o $(INCDIR) -c ReadLevels.c; \
	$(CC) $(CFLAGS) -o ReadLevels ReadLevels.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o SubsetPoint.o $(INCDIR) -c SubsetPoint.c; \
	$(CC) $(CFLAGS) -o SubsetPoint SubsetPoint.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	echo " "; echo " "; \
	echo " ---- Making *.c samples for grid ----"; \
	$(CC) $(CFLAGS) -o SetupGrid.o $(INCDIR) -c SetupGrid.c; \
	$(CC) $(CFLAGS) -o SetupGrid SetupGrid.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o DefineGDflds.o $(INCDIR) -c DefineGDflds.c; \
	$(CC) $(CFLAGS) -o DefineGDflds DefineGDflds.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o WriteGDflds.o $(INCDIR) -c WriteGDflds.c; \
	$(CC) $(CFLAGS) -o WriteGDflds WriteGDflds.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o InquireGrid.o $(INCDIR) -c InquireGrid.c; \
	$(CC) $(CFLAGS) -o InquireGrid InquireGrid.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o ReadGDflds.o $(INCDIR) -c ReadGDflds.c; \
	$(CC) $(CFLAGS) -o ReadGDflds ReadGDflds.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o SubsetGrid.o $(INCDIR) -c SubsetGrid.c; \
	$(CC) $(CFLAGS) -o SubsetGrid SubsetGrid.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o WriteDimscaleGrid.o $(INCDIR) -c WriteDimscaleGrid.c; \
	$(CC) $(CFLAGS) -o WriteDimscaleGrid WriteDimscaleGrid.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o; \
	$(CC) $(CFLAGS) -o ReadDimscaleGrid.o $(INCDIR) -c ReadDimscaleGrid.c; \
	$(CC) $(CFLAGS) -o ReadDimscaleGrid ReadDimscaleGrid.o $(LIBDIR) $(CEXTRAL); \
	$(RM) $(RMFLAGS) *.o;

clean:
	rm -f *.o SubsetPoint ReadLevels UpdateLevels 
	rm -f WriteLevels DefineLevels SetupPoint
	rm -f SubsetGrid ReadGDflds InquireGrid WriteGDflds DefineGDflds
	rm -f SetupGrid SubsetSwath ReadFields InquireSwath AppendField
	rm -f WriteFields DefineFields SetupSwath
	rm -f ReadDimscaleGrid WriteDimscaleGrid
	rm -f ReadDimscaleSwath WriteDimscaleSwath








