# ACPICA kernel integration Makefile
ACPI_SRC = $(wildcard dispatcher/*.c executer/*.c namespace/*.c tables/*.c utilities/*.c)
ACPI_INC = -I. -Iinclude

acpica.o: $(ACPI_SRC)
	$(CC) $(CFLAGS) $(ACPI_INC) -c $^ -o $@
