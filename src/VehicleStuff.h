// VEHICLE STUFF //////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

#define VEHICLE_LIST_ID_START		400
#define VEHICLE_LIST_SIZE			212

#define VEHICLE_CLASS_NONE			-1
#define VEHICLE_CLASS_CAR			0
#define VEHICLE_CLASS_CAR_FAST		1
#define VEHICLE_CLASS_HEAVY			2
#define VEHICLE_CLASS_HELI			3
#define VEHICLE_CLASS_AIRPLANE		4
#define VEHICLE_CLASS_BIKE			5
#define VEHICLE_CLASS_BOAT			6
#define VEHICLE_CLASS_MINI			7
#define VEHICLE_CLASS_TRAILER		8
#define VEHICLE_CLASS_COUNT			9	/* # of classes */

struct vehicle_entry {
	int			id;			// model id
	int			classId;	// class id
	const char	*name;		// vehicle name
	int			passengers; // total passenger seats, 0-10
};

extern vehicle_entry VehicleList[VEHICLE_LIST_SIZE];