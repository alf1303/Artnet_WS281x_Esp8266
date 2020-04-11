
char* convertModes(int mod) {
    switch (mod)
    {
    case 0:
        return "WIFI";
        break;
    case 1:
        return "LAN";
        break;
    case 2:
        return "AutoMode";
        break;
    default:
        return "ErrorMode";
        break;
    }
}

char* convertAutoModes(int automod)  {
    switch (automod)
    {
    case 0:
        return "CHASE";
        break;
    case 1:
        return "WHITE";
        break;
    case 2:
        return "RED";
        break;
    case 3:
        return "GREEN";
        break;
    case 4:
        return "BLUE";
        break;
    default:
        return "ErrorMode";
        break;
    }
}
