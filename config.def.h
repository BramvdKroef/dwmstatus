

/*
 *  Argument for the mailmonitor_ctl function.
 *
 *  - path      The Maildir mailbox to monitor
 *  - emails    Stores the number of emails.
 */
struct mailmonitor_data maildata = {
  // path                         emails
  "/home/[user]/mail/INBOX/new",   0};

/*
 * Argument for weather_ctl function
 */
struct weather_data weatherdata = {
  // url 
  "https://weather.gc.ca/rss/city/[city].xml",
  // Xpath to weather
  "/atom:feed/atom:entry/atom:category[@term=\"Current Conditions\"]/../atom:title/text()",
  // xpath to warnings
  "/atom:feed/atom:entry/atom:category[@term=\"Warnings and Watches\"]/../atom:title/text()"
};

/*
 * Argument for net_if_ctl function.
 *
 * - interface    Network interface to monitor
 * - tx           Stores outgoing bytes
 * - rx           Stores incoming bytes
 */
struct net_if_data netif = {
  // interface    tx  rx
     "eth0",      0,  0
};
  
/*  
 * Declare segments of the status bar.
 *
 * - len           Length (in bytes) to allocate in the status bar.
 * - format        printf format to use when printing to the status bar.
 * - function      Function to use to generate data.
 * - null          Will be filled with pointer to this segment of the status
 *                 bar. 
 * - function arg  Argument to pass to the function.
 */
size_t ctl_n = 5;
CtlData ctl_p[5] = {
  // len  format                 function         null     function arg
  {  8,   "%s",                  weather_ctl,     NULL,    &weatherdata},
  {  9,   "RAM: %3.0f%%",        ram_ctl,         NULL,    NULL},
  {  9,   "Email: %02d",         mailmonitor_ctl, NULL,    &maildata},
  {  23,  "%e %b %Y %a | %k:%M", datetime_ctl,    NULL,    NULL},
  {  17,  "↑ %04d/s ↓ %04d/s",   net_if_ctl,      NULL,    &netif}
};
