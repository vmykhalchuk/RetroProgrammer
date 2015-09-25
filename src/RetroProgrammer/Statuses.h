#ifndef Statuses_h
#define Statuses_h

  #include <Arduino.h>

  #define ERR(err) err

  #define initStatus() statusRes=0;
  #define checkStatus() if (statusRes != 0) return;
  #define checkStatusV(v) if (statusRes != 0) return v;
  #define checkOverrideStatus(err) if (statusRes != 0) { logDebugB("@pr@",statusRes); statusRes = err; return; }
  #define checkOverrideStatusV(err, v) if (statusRes != 0) { logDebugB("@pr@",statusRes); statusRes = err; return v; }
  #define returnStatus(err) { statusRes = err; return; }
  #define returnStatusV(err, v) { statusRes = err; return v; }
  #define returnStatusOK() { statusRes = 0; return; }
  #define returnStatusOKV(v) { statusRes = 0; return v; }

#endif
