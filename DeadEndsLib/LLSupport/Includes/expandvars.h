enum ExpandErrorCode
  {
    EV_OK = 0,
    EV_NOT_FOUND,
    EV_SYNTAX
  };

enum ExpandErrorCode
expandVariables (CString input, String *output, CString *details);
