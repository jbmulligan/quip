#include "quip_config.h"

#include "quip_prot.h"

#include "polh_dev.h"

#define MAX_STRING_LEN		256	// overkill?

int set_polh_angles(QSP_ARG_DECL  Ph_Cmd_Code code)
{
	char angles[MAX_STRING_LEN];	// BUG - check for overrun?
	float az, el, rl;

	az = (float)HOW_MUCH("azimuth reference angle");
	el = (float)HOW_MUCH("elevation reference angle");
	rl = (float)HOW_MUCH("roll reference angle");

	/* FIXME - we should check these values to make sure they are
	 *         reasonable ...
	 */
	snprintf(angles,MAX_STRING_LEN, "%3.2f,%3.2f,%3.2f", az, el, rl);

	return send_polh_cmd(code, angles);
}

int set_polh_filter(QSP_ARG_DECL  Ph_Cmd_Code code)
{
	char filter[MAX_STRING_LEN];	// BUG - overrun?
	float f, flow, fhigh, factor;

#define MIN_SENSITIVITY_FILTER	0
#define MAX_SENSITIVITY_FILTER	1	
#define MIN_FACTOR_FILTER	0
#define MAX_FACTOR_FILTER	1
#define MIN_FLOW_FILTER		0
#define MAX_FHIGH_FILTER	1

	f = (float)HOW_MUCH("sensitivity of filter");
	if(f < MIN_SENSITIVITY_FILTER || f > MAX_SENSITIVITY_FILTER) {
		snprintf(ERROR_STRING,LLEN, "Requested sensitivity filter %1.3f, should be between 0 and 1", f);
		warn(ERROR_STRING);
		return(-1);
	}

	flow = (float)HOW_MUCH("maximum allowable filtering");
	fhigh = (float)HOW_MUCH("minimum allowable filtering");

	if( flow < MIN_FLOW_FILTER || flow > fhigh ) {
		snprintf(ERROR_STRING,LLEN, "Requested max. filter %1.3f, should be between 0 and min. filter %1.3f", flow, fhigh);
		warn(ERROR_STRING);
		return(-1);
	}

	if( fhigh < flow || fhigh > MAX_FHIGH_FILTER ) {
		snprintf(ERROR_STRING,LLEN, "Requested min. filter %1.3f, should be between max. filter %1.3f and 1", fhigh, flow);
		warn(ERROR_STRING);
		return(-1);
	}
	
	factor = (float)HOW_MUCH("maximum allowable transition rate");
	if(factor < MIN_FACTOR_FILTER || factor > MAX_FACTOR_FILTER) {
		snprintf(ERROR_STRING,LLEN, "Requested transition rate %1.3f, should be between 0 and 1", factor);
		warn(ERROR_STRING);
		return(-1);
	}
	
	snprintf(filter,MAX_STRING_LEN, "%1.3f,%1.3f,%1.3f,%1.3f", f, flow, fhigh, factor);

	return send_polh_cmd(code, filter);
}

int ask_env(QSP_ARG_DECL  const char* name, const char* type, float* max_value, float* min_value, float sys_max, float sys_min)
{
	char ask_string[MAX_STRING_LEN];	// BUG - overrun?
	
	snprintf(ask_string,MAX_STRING_LEN,"minimum %s value for %s",name, type);
	*min_value = (float)HOW_MUCH(ask_string);
	if( *min_value < sys_min || *min_value > sys_max ) {
		snprintf(ERROR_STRING,LLEN,"bad minimum %s %f specified, must be between %f and %f",
			name, *min_value, sys_min, sys_max);
		warn(ERROR_STRING);
		return(-1);
	}

	snprintf(ask_string,MAX_STRING_LEN,"maximum %s value for %s",name, type);
	*max_value = (float)HOW_MUCH(ask_string);
	if( *max_value < *min_value || *max_value > sys_max ) {
		snprintf(ERROR_STRING,LLEN,"bad maximum %s %f specified, must be between %f and %f",
			name, *max_value, *min_value, sys_max);
		warn(ERROR_STRING);
		return(-1);
	}

	return(0);
}
