// BeaconGTKDemo.c
// Matthieu Vigne <matthieu.vigne@laposte.net>
//
// This is a demo code for the beacon, which uses GTK to render in a graphical window the current beacon status.

#include <stdio.h>
#include <gtk/gtk.h>

#include "BeaconDriver.h"
#include "I2C-Wrapper.h"

// Graphical element declaration.
#define DRAWING_SIZE 500
#define N_SENSORS 9
#define N_REGISTERS 6
gchar *registerList[N_REGISTERS] = {"WHO_AM_I", "IR_FREQUENCY", "IR_RAW_DATA1", "IR_RAW_DATA2", "IR_ROBOT1_POS", "IR_ROBOT2_POS"};
unsigned char registerData[N_REGISTERS];

// Processed data recieved from driver
int frequency;
int rawData;
int robotPos[2];
int robotSize[2];

GtkWidget *labelDecimal[N_REGISTERS], *labelHexa[N_REGISTERS], *labelBinary[N_REGISTERS];

// The beacon driver structure.
Beacon beacon;

// Initialize I2C connexion with beacon.
gboolean initI2CConnexion()
{
	// Open i2c port.
	int dev = i2c_open("/dev/i2c-1");
	if(dev < 0)
	{
		printf("Error opening i2c port: is i2c enabled?\n");
		return FALSE;
	}
	return IR_init(&beacon, dev, IR_RECEIVER_ADDRESS);
}

// Update beacon drawing.
gboolean redraw(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	cairo_t *context = gdk_cairo_create(widget->window);
	// Draw PCB
	cairo_set_source_rgb(context, 0,0.5,0);
	cairo_new_path(context);
	double circleRadius = DRAWING_SIZE / 3.0;
	cairo_arc(context, DRAWING_SIZE / 2.0, DRAWING_SIZE / 2.0, circleRadius, 0, 2 * G_PI);
	cairo_fill(context);
	cairo_set_source_rgb(context, 0.9,0.7,0);
	cairo_arc(context, DRAWING_SIZE / 2.0 + 0.85 * circleRadius, DRAWING_SIZE / 2.0 + 0.15 * circleRadius, circleRadius / 30.0, 0, 2 * G_PI);
	cairo_fill(context);
	// Draw all beacon, move them according to the choosen frequency.
	cairo_save(context);
	cairo_translate(context, DRAWING_SIZE / 2.0, DRAWING_SIZE / 2.0);
	double rectangleSize = circleRadius * 0.1;
	if(frequency == IR_FREQUENCY_38k)
		cairo_rotate(context, - G_PI / N_SENSORS);
	for(int i = 0; i < N_SENSORS; i++)
	{
		cairo_new_path(context);
		cairo_rectangle(context, circleRadius - 2 * rectangleSize, -rectangleSize, rectangleSize, 2 * rectangleSize);
		cairo_set_source_rgb(context, 0,0,0);
		cairo_fill(context);
		// If detected, draw blue circle on beacon.
		if((rawData >> i) % 2 == 1)
		{
			cairo_new_path(context);
			cairo_arc(context, circleRadius - rectangleSize, 0, circleRadius / 30.0, 0, 2 * G_PI);
			cairo_set_source_rgb(context, 0,0,1);
			cairo_fill(context);
		}
		cairo_rotate(context, - 2 * G_PI / N_SENSORS);
	}
	cairo_restore(context);
	// Draw robots.
	cairo_set_line_width(context, 5.0);

	for(int i = 0; i < 2; i++)
	{
		if(robotSize[i] > 0)
		{
			cairo_set_source_rgb(context, 0.8 * ((i+1)%2),0.8 * i,0);
			double arcAngle = robotSize[i] * 2 * G_PI / N_SENSORS;
			double arcStart = robotPos[i] * G_PI / 180.0 - arcAngle / 2.0;
			cairo_new_path(context);
			cairo_arc_negative(context, DRAWING_SIZE / 2.0, DRAWING_SIZE / 2.0, circleRadius * (1.2 + 0.1 * i), 2 * G_PI - arcStart, 2 * G_PI - (arcStart + arcAngle));
			cairo_stroke(context);
		}
	}
	return FALSE;
}

// Convert a single byte into a 8-character binary stream (because printf can't do it on its own).
gchar *byteToBinary(unsigned char value)
{
	gchar string[9];
	for(int i = 7; i > -1; i--)
	{
		string[i] = '0' + value % 2;
		value = value / 2;
	}
	string[8] = '\0';
	return g_strdup(string);
}

// Ask the beacon for new data and update window display.
// This function is automatically called periodically.
gboolean getNewData(gpointer widget)
{
	// Get all registers.
	for(int i = 0; i < N_REGISTERS; i++)
		registerData[i] = i2c_readRegister(beacon.port, beacon.address, i);
	// Get frequency and raw data.
	frequency = IR_getFrequency(&beacon);
	rawData = IR_getRawValue(beacon);
	IR_getObstacle(beacon, 0, &robotSize[0], &robotPos[0]);
	IR_getObstacle(beacon, 1, &robotSize[1], &robotPos[1]);

	// Update labels.
	for(int i = 0; i < N_REGISTERS; i++)
	{
		gchar *value = g_strdup_printf("%d", registerData[i]);
		gtk_label_set_text(GTK_LABEL(labelDecimal[i]), value);
		g_free(value);
		value = g_strdup_printf("0x%02x", registerData[i]);
		gtk_label_set_text(GTK_LABEL(labelHexa[i]), value);
		g_free(value);
		value = byteToBinary(registerData[i]);
		gtk_label_set_text(GTK_LABEL(labelBinary[i]), value);
		g_free(value);
	}

	// Queue redraw of drawing area.
	gtk_widget_queue_draw(widget);
	return TRUE;
}

int main(int argc, char **argv)
{
	if(!initI2CConnexion())
	{
		printf("Error connecting to beacon: check wiring.\n");
		return -1;
	}
	// Clear registers
	for(int i = 0; i < N_REGISTERS; i++)
		registerData[i] = 0;

	// Create window.
	gtk_init (&argc, &argv);
	// Create a window with a gtk drawing area inside.
	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Beacon demo");
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect(window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
	// Add table inside window.
	GtkWidget *table = gtk_table_new (1 + N_REGISTERS,5,FALSE);
	gtk_table_set_row_spacings (GTK_TABLE(table),10);
	gtk_table_set_col_spacings (GTK_TABLE(table),10);
	gtk_container_add (GTK_CONTAINER (window), table);
	// On right column, add labels to display all the registers.
	GtkWidget *title = gtk_label_new("Register List");
	gtk_table_attach_defaults (GTK_TABLE(table), title, 0,3,0,1);
	GtkWidget *name = gtk_label_new("Register Name");
	gtk_table_attach_defaults (GTK_TABLE(table), name, 0,1,1,2);
	GtkWidget *decimal = gtk_label_new("Decimal");
	gtk_table_attach_defaults (GTK_TABLE(table), decimal, 1,2,1,2);
	GtkWidget *hexa = gtk_label_new("Hexadecimal");
	gtk_table_attach_defaults (GTK_TABLE(table), hexa, 2,3,1,2);
	GtkWidget *binary = gtk_label_new("Binary");
	gtk_table_attach_defaults (GTK_TABLE(table), binary, 3,4,1,2);

	GtkWidget *registerNames[N_REGISTERS];
	for(int i = 0; i < N_REGISTERS; i++)
	{
		registerNames[i] = gtk_label_new(registerList[i]);
		labelDecimal[i] = gtk_label_new("0");
		labelHexa[i] = gtk_label_new("0");
		labelBinary[i] = gtk_label_new("0");
		gtk_table_attach_defaults (GTK_TABLE(table), registerNames[i], 0,1,i+2,i+3);
		gtk_table_attach_defaults (GTK_TABLE(table), labelDecimal[i], 1,2,i+2,i+3);
		gtk_table_attach_defaults (GTK_TABLE(table), labelHexa[i], 2,3,i+2,i+3);
		gtk_table_attach_defaults (GTK_TABLE(table), labelBinary[i], 3,4,i+2,i+3);
	}

	// Add drawing area
	GtkWidget *draw = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), DRAWING_SIZE, DRAWING_SIZE);
	g_signal_connect(G_OBJECT(draw), "expose-event", G_CALLBACK(redraw), NULL);
	gtk_table_attach_defaults (GTK_TABLE(table), draw, 4,5,0, 3 + N_REGISTERS);

	// Add timeout to ask the beacon for new data.
	g_timeout_add(50, getNewData, draw);

	// Run application.
	gtk_widget_show_all(window);
	gtk_main ();
	return 0;
}

