# This file was autogenerated from ../../../lib/packagekit-glib2/pk-enum.c by enum-converter.py

class PackageKitEnum:
	exit = ( "unknown", "success", "failed", "cancelled", "key-required", "eula-required", "media-change-required", "killed", "need-untrusted", )
	status = ( "unknown", "wait", "setup", "running", "query", "info", "refresh-cache", "remove", "download", "install", "update", "cleanup", "obsolete", "dep-resolve", "sig-check", "rollback", "test-commit", "commit", "request", "finished", "cancel", "download-repository", "download-packagelist", "download-filelist", "download-changelog", "download-group", "download-updateinfo", "repackaging", "loading-cache", "scan-applications", "generate-package-list", "waiting-for-lock", "waiting-for-auth", "scan-process-list", "check-executable-files", "check-libraries", "copy-files", )
	role = ( "unknown", "cancel", "get-depends", "get-details", "get-files", "get-packages", "get-repo-list", "get-requires", "get-update-detail", "get-updates", "install-files", "install-packages", "install-signature", "refresh-cache", "remove-packages", "repo-enable", "repo-set-data", "resolve", "rollback", "search-details", "search-file", "search-group", "search-name", "update-packages", "update-system", "what-provides", "accept-eula", "download-packages", "get-distro-upgrades", "get-categories", "get-old-transactions", "simulate-install-files", "simulate-install-packages", "simulate-remove-packages", "simulate-update-packages", )
	error = ( "unknown", "out-of-memory", "no-cache", "no-network", "not-supported", "internal-error", "gpg-failure", "filter-invalid", "package-id-invalid", "transaction-error", "transaction-cancelled", "package-not-installed", "package-not-found", "package-already-installed", "package-download-failed", "group-not-found", "group-list-invalid", "dep-resolution-failed", "create-thread-failed", "repo-not-found", "cannot-remove-system-package", "process-kill", "failed-initialization", "failed-finalise", "failed-config-parsing", "cannot-cancel", "cannot-get-lock", "no-packages-to-update", "cannot-write-repo-config", "local-install-failed", "bad-gpg-signature", "missing-gpg-signature", "cannot-install-source-package", "repo-configuration-error", "no-license-agreement", "file-conflicts", "package-conflicts", "repo-not-available", "invalid-package-file", "package-install-blocked", "package-corrupt", "all-packages-already-installed", "file-not-found", "no-more-mirrors-to-try", "no-distro-upgrade-data", "incompatible-architecture", "no-space-on-device", "media-change-required", "not-authorized", "update-not-found", "cannot-install-repo-unsigned", "cannot-update-repo-unsigned", "cannot-get-filelist", "cannot-get-requires", "cannot-disable-repository", "restricted-download", "package-failed-to-configure", "package-failed-to-build", "package-failed-to-install", "package-failed-to-remove", "failed-due-to-running-process", "package-database-changed", "provide-type-not-supported", "install-root-invalid", )
	restart = ( "unknown", "none", "system", "session", "application", "security-system", "security-session", )
	message = ( "unknown", "broken-mirror", "connection-refused", "parameter-invalid", "priority-invalid", "backend-error", "daemon-error", "cache-being-rebuilt", "untrusted-package", "newer-package-exists", "could-not-find-package", "config-files-changed", "package-already-installed", "autoremove-ignored", "repo-metadata-download-failed", "repo-for-developers-only", "other-updates-held-back", )
	filter = ( "unknown", "none", "installed", "~installed", "devel", "~devel", "gui", "~gui", "free", "~free", "visible", "~visible", "supported", "~supported", "basename", "~basename", "newest", "~newest", "arch", "~arch", "source", "~source", "collections", "~collections", "application", "~application", )
	group = ( "unknown", "accessibility", "accessories", "education", "games", "graphics", "internet", "office", "other", "programming", "multimedia", "system", "desktop-gnome", "desktop-kde", "desktop-xfce", "desktop-other", "publishing", "servers", "fonts", "admin-tools", "legacy", "localization", "virtualization", "power-management", "security", "communication", "network", "maps", "repos", "science", "documentation", "electronics", "collections", "vendor", "newest", )
	update_state = ( "unknown", "testing", "unstable", "stable", )
	info = ( "unknown", "installed", "available", "low", "normal", "important", "security", "bugfix", "enhancement", "blocked", "downloading", "updating", "installing", "removing", "cleanup", "obsoleting", "collection-installed", "collection-available", "finished", "reinstalling", "downgrading", "preparing", "decompressing", )
	sig_type = ( "unknown", "gpg", )
	upgrade = ( "unknown", "stable", "unstable", )
	provides = ( "unknown", "any", "modalias", "codec", "mimetype", "driver", "font", "postscript-driver", )
	network = ( "unknown", "offline", "online", "wired", "wifi", "mobile", )
	free_licenses = ( "unknown", "AAL", "Adobe", "ADSL", "AFL", "AGPLv1", "AMDPLPA", "AML", "AMPAS BSD", "APSL 2.0", "ARL", "Arphic", "Artistic 2.0", "Artistic clarified", "ASL 1.0", "ASL 1.1", "ASL 2.0", "Baekmuk", "BeOpen", "BitTorrent", "Boost", "BSD", "BSD Protection", "BSD with advertising", "CATOSL", "CC0", "CC-BY", "CC-BY-SA", "CDDL", "CDL", "CeCILL", "CeCILL-B", "CeCILL-C", "CNRI", "Condor", "Copyright only", "CPAL", "CPL", "Crystal Stacker", "DOC", "DSL", "dvipdfm", "ECL 1.0", "ECL 2.0", "eCos", "EFL 2.0", "Entessa", "EPL", "ERPL", "EUPL 1.1", "Eurosym", "EU Datagrid", "Fair", "FBSDDL", "Free Art", "FTL", "GeoGratis", "GFDL", "Giftware", "GL2PS", "Glide", "gnuplot", "GPLv1", "GPLv2", "GPLv2 or Artistic", "GPLv2+", "GPLv2+ or Artistic", "GPLv2+ with exceptions", "GPLv2 with exceptions", "GPLv3", "GPLv3+", "GPLv3+ with exceptions", "GPLv3 with exceptions", "GPL+", "GPL+ or Artistic", "GPL+ with exceptions", "IBM", "IEEE", "IJG", "ImageMagick", "iMatix", "Imlib2", "Intel ACPI", "Interbase", "IPA", "ISC", "Jabber", "JasPer", "JPython", "Knuth", "LBNL BSD", "LDPL", "LGPLv2", "LGPLv2+", "LGPLv2+ or Artistic", "LGPLv2+ with exceptions", "LGPLv2 with exceptions", "LGPLv3", "LGPLv3+", "LGPLv3+ with exceptions", "LGPLv3 with exceptions", "Liberation", "libtiff", "LLGPL", "Logica", "LPL", "LPPL", "mecab-ipadic", "MirOS", "MIT", "MIT with advertising", "mod_macro", "Motosoto", "mplus", "MPLv1.0", "MPLv1.1", "MS-PL", "MS-RL", "Naumen", "NCSA", "NetCDF", "Netscape", "Newmat", "NGPL", "Nokia", "NOSL", "Noweb", "OAL", "OFL", "OFSFDL", "OML", "OpenLDAP", "OpenPBS", "OpenSSL", "OReilly", "OSL 1.0", "OSL 1.1", "OSL 2.0", "OSL 2.1", "OSL 3.0", "Phorum", "PHP", "Plexus", "PostgreSQL", "psutils", "PTFL", "Public Domain", "Public Use", "Python", "Qhull", "QPL", "Rdisc", "RiceBSD", "Romio", "RPSL", "Ruby", "Saxpath", "SCEA", "SCRIP", "Sendmail", "SISSL", "Sleepycat", "SLIB", "SNIA", "SPL", "STIX", "TCL", "Teeworlds", "TMate", "TOSL", "TPL", "UCD", "Vim", "VNLSL", "VOSTROM", "VSL", "W3C", "Wadalab", "Webmin", "WTFPL", "wxWidgets", "XANO", "Xerox", "xinetd", "XSkat", "YPLv1.1", "Zend", "zlib", "zlib with acknowledgement", "ZPLv1.0", "ZPLv2.0", "ZPLv2.1", )
	media_type = ( "unknown", "cd", "dvd", "disc", )
	authorize_type = ( "unknown", "yes", "no", "interactive", )

# Constants

AUTHORIZE_INTERACTIVE = "interactive"
AUTHORIZE_NO = "no"
AUTHORIZE_UNKNOWN = "unknown"
AUTHORIZE_YES = "yes"
DISTRO_UPGRADE_STABLE = "stable"
DISTRO_UPGRADE_UNKNOWN = "unknown"
DISTRO_UPGRADE_UNSTABLE = "unstable"
ERROR_ALL_PACKAGES_ALREADY_INSTALLED = "all-packages-already-installed"
ERROR_BAD_GPG_SIGNATURE = "bad-gpg-signature"
ERROR_CANNOT_CANCEL = "cannot-cancel"
ERROR_CANNOT_DISABLE_REPOSITORY = "cannot-disable-repository"
ERROR_CANNOT_GET_FILELIST = "cannot-get-filelist"
ERROR_CANNOT_GET_LOCK = "cannot-get-lock"
ERROR_CANNOT_GET_REQUIRES = "cannot-get-requires"
ERROR_CANNOT_INSTALL_REPO_UNSIGNED = "cannot-install-repo-unsigned"
ERROR_CANNOT_INSTALL_SOURCE_PACKAGE = "cannot-install-source-package"
ERROR_CANNOT_REMOVE_SYSTEM_PACKAGE = "cannot-remove-system-package"
ERROR_CANNOT_UPDATE_REPO_UNSIGNED = "cannot-update-repo-unsigned"
ERROR_CANNOT_WRITE_REPO_CONFIG = "cannot-write-repo-config"
ERROR_CREATE_THREAD_FAILED = "create-thread-failed"
ERROR_DEP_RESOLUTION_FAILED = "dep-resolution-failed"
ERROR_FAILED_CONFIG_PARSING = "failed-config-parsing"
ERROR_FAILED_FINALISE = "failed-finalise"
ERROR_FAILED_INITIALIZATION = "failed-initialization"
ERROR_FILE_CONFLICTS = "file-conflicts"
ERROR_FILE_NOT_FOUND = "file-not-found"
ERROR_FILTER_INVALID = "filter-invalid"
ERROR_GPG_FAILURE = "gpg-failure"
ERROR_GROUP_LIST_INVALID = "group-list-invalid"
ERROR_GROUP_NOT_FOUND = "group-not-found"
ERROR_INCOMPATIBLE_ARCHITECTURE = "incompatible-architecture"
ERROR_INSTALL_ROOT_INVALID = "install-root-invalid"
ERROR_INTERNAL_ERROR = "internal-error"
ERROR_INVALID_PACKAGE_FILE = "invalid-package-file"
ERROR_LOCAL_INSTALL_FAILED = "local-install-failed"
ERROR_MEDIA_CHANGE_REQUIRED = "media-change-required"
ERROR_MISSING_GPG_SIGNATURE = "missing-gpg-signature"
ERROR_NOT_AUTHORIZED = "not-authorized"
ERROR_NOT_SUPPORTED = "not-supported"
ERROR_NO_CACHE = "no-cache"
ERROR_NO_DISTRO_UPGRADE_DATA = "no-distro-upgrade-data"
ERROR_NO_LICENSE_AGREEMENT = "no-license-agreement"
ERROR_NO_MORE_MIRRORS_TO_TRY = "no-more-mirrors-to-try"
ERROR_NO_NETWORK = "no-network"
ERROR_NO_PACKAGES_TO_UPDATE = "no-packages-to-update"
ERROR_NO_SPACE_ON_DEVICE = "no-space-on-device"
ERROR_OOM = "out-of-memory"
ERROR_PACKAGE_ALREADY_INSTALLED = "package-already-installed"
ERROR_PACKAGE_CONFLICTS = "package-conflicts"
ERROR_PACKAGE_CORRUPT = "package-corrupt"
ERROR_PACKAGE_DATABASE_CHANGED = "package-database-changed"
ERROR_PACKAGE_DOWNLOAD_FAILED = "package-download-failed"
ERROR_PACKAGE_FAILED_TO_BUILD = "package-failed-to-build"
ERROR_PACKAGE_FAILED_TO_CONFIGURE = "package-failed-to-configure"
ERROR_PACKAGE_FAILED_TO_INSTALL = "package-failed-to-install"
ERROR_PACKAGE_FAILED_TO_REMOVE = "package-failed-to-remove"
ERROR_PACKAGE_ID_INVALID = "package-id-invalid"
ERROR_PACKAGE_INSTALL_BLOCKED = "package-install-blocked"
ERROR_PACKAGE_NOT_FOUND = "package-not-found"
ERROR_PACKAGE_NOT_INSTALLED = "package-not-installed"
ERROR_PROCESS_KILL = "process-kill"
ERROR_PROVIDE_TYPE_NOT_SUPPORTED = "provide-type-not-supported"
ERROR_REPO_CONFIGURATION_ERROR = "repo-configuration-error"
ERROR_REPO_NOT_AVAILABLE = "repo-not-available"
ERROR_REPO_NOT_FOUND = "repo-not-found"
ERROR_RESTRICTED_DOWNLOAD = "restricted-download"
ERROR_TRANSACTION_CANCELLED = "transaction-cancelled"
ERROR_TRANSACTION_ERROR = "transaction-error"
ERROR_UNKNOWN = "unknown"
ERROR_UPDATE_FAILED_DUE_TO_RUNNING_PROCESS = "failed-due-to-running-process"
ERROR_UPDATE_NOT_FOUND = "update-not-found"
EXIT_CANCELLED = "cancelled"
EXIT_EULA_REQUIRED = "eula-required"
EXIT_FAILED = "failed"
EXIT_KEY_REQUIRED = "key-required"
EXIT_KILLED = "killed"
EXIT_MEDIA_CHANGE_REQUIRED = "media-change-required"
EXIT_NEED_UNTRUSTED = "need-untrusted"
EXIT_SUCCESS = "success"
EXIT_UNKNOWN = "unknown"
FILTER_APPLICATION = "application"
FILTER_ARCH = "arch"
FILTER_BASENAME = "basename"
FILTER_COLLECTIONS = "collections"
FILTER_DEVELOPMENT = "devel"
FILTER_FREE = "free"
FILTER_GUI = "gui"
FILTER_INSTALLED = "installed"
FILTER_NEWEST = "newest"
FILTER_NONE = "none"
FILTER_NOT_APPLICATION = "~application"
FILTER_NOT_ARCH = "~arch"
FILTER_NOT_BASENAME = "~basename"
FILTER_NOT_COLLECTIONS = "~collections"
FILTER_NOT_DEVELOPMENT = "~devel"
FILTER_NOT_FREE = "~free"
FILTER_NOT_GUI = "~gui"
FILTER_NOT_INSTALLED = "~installed"
FILTER_NOT_NEWEST = "~newest"
FILTER_NOT_SOURCE = "~source"
FILTER_NOT_SUPPORTED = "~supported"
FILTER_NOT_VISIBLE = "~visible"
FILTER_SOURCE = "source"
FILTER_SUPPORTED = "supported"
FILTER_UNKNOWN = "unknown"
FILTER_VISIBLE = "visible"
GROUP_ACCESSIBILITY = "accessibility"
GROUP_ACCESSORIES = "accessories"
GROUP_ADMIN_TOOLS = "admin-tools"
GROUP_COLLECTIONS = "collections"
GROUP_COMMUNICATION = "communication"
GROUP_DESKTOP_GNOME = "desktop-gnome"
GROUP_DESKTOP_KDE = "desktop-kde"
GROUP_DESKTOP_OTHER = "desktop-other"
GROUP_DESKTOP_XFCE = "desktop-xfce"
GROUP_DOCUMENTATION = "documentation"
GROUP_EDUCATION = "education"
GROUP_ELECTRONICS = "electronics"
GROUP_FONTS = "fonts"
GROUP_GAMES = "games"
GROUP_GRAPHICS = "graphics"
GROUP_INTERNET = "internet"
GROUP_LEGACY = "legacy"
GROUP_LOCALIZATION = "localization"
GROUP_MAPS = "maps"
GROUP_MULTIMEDIA = "multimedia"
GROUP_NETWORK = "network"
GROUP_NEWEST = "newest"
GROUP_OFFICE = "office"
GROUP_OTHER = "other"
GROUP_POWER_MANAGEMENT = "power-management"
GROUP_PROGRAMMING = "programming"
GROUP_PUBLISHING = "publishing"
GROUP_REPOS = "repos"
GROUP_SCIENCE = "science"
GROUP_SECURITY = "security"
GROUP_SERVERS = "servers"
GROUP_SYSTEM = "system"
GROUP_UNKNOWN = "unknown"
GROUP_VENDOR = "vendor"
GROUP_VIRTUALIZATION = "virtualization"
INFO_AVAILABLE = "available"
INFO_BLOCKED = "blocked"
INFO_BUGFIX = "bugfix"
INFO_CLEANUP = "cleanup"
INFO_COLLECTION_AVAILABLE = "collection-available"
INFO_COLLECTION_INSTALLED = "collection-installed"
INFO_DECOMPRESSING = "decompressing"
INFO_DOWNGRADING = "downgrading"
INFO_DOWNLOADING = "downloading"
INFO_ENHANCEMENT = "enhancement"
INFO_FINISHED = "finished"
INFO_IMPORTANT = "important"
INFO_INSTALLED = "installed"
INFO_INSTALLING = "installing"
INFO_LOW = "low"
INFO_NORMAL = "normal"
INFO_OBSOLETING = "obsoleting"
INFO_PREPARING = "preparing"
INFO_REINSTALLING = "reinstalling"
INFO_REMOVING = "removing"
INFO_SECURITY = "security"
INFO_UNKNOWN = "unknown"
INFO_UPDATING = "updating"
LICENSE_AAL = "AAL"
LICENSE_ADOBE = "Adobe"
LICENSE_ADSL = "ADSL"
LICENSE_AFL = "AFL"
LICENSE_AGPLV1 = "AGPLv1"
LICENSE_AMDPLPA = "AMDPLPA"
LICENSE_AML = "AML"
LICENSE_AMPAS_BSD = "AMPAS BSD"
LICENSE_APSL_2_DOT_0 = "APSL 2.0"
LICENSE_ARL = "ARL"
LICENSE_ARPHIC = "Arphic"
LICENSE_ARTISTIC_2_DOT_0 = "Artistic 2.0"
LICENSE_ARTISTIC_CLARIFIED = "Artistic clarified"
LICENSE_ASL_1_DOT_0 = "ASL 1.0"
LICENSE_ASL_1_DOT_1 = "ASL 1.1"
LICENSE_ASL_2_DOT_0 = "ASL 2.0"
LICENSE_BAEKMUK = "Baekmuk"
LICENSE_BEOPEN = "BeOpen"
LICENSE_BITTORRENT = "BitTorrent"
LICENSE_BOOST = "Boost"
LICENSE_BSD = "BSD"
LICENSE_BSD_PROTECTION = "BSD Protection"
LICENSE_BSD_WITH_ADVERTISING = "BSD with advertising"
LICENSE_CATOSL = "CATOSL"
LICENSE_CC0 = "CC0"
LICENSE_CC_BY = "CC-BY"
LICENSE_CC_BY_SA = "CC-BY-SA"
LICENSE_CDDL = "CDDL"
LICENSE_CDL = "CDL"
LICENSE_CECILL = "CeCILL"
LICENSE_CECILL_B = "CeCILL-B"
LICENSE_CECILL_C = "CeCILL-C"
LICENSE_CNRI = "CNRI"
LICENSE_CONDOR = "Condor"
LICENSE_COPYRIGHT_ONLY = "Copyright only"
LICENSE_CPAL = "CPAL"
LICENSE_CPL = "CPL"
LICENSE_CRYSTAL_STACKER = "Crystal Stacker"
LICENSE_DOC = "DOC"
LICENSE_DSL = "DSL"
LICENSE_DVIPDFM = "dvipdfm"
LICENSE_ECL_1_DOT_0 = "ECL 1.0"
LICENSE_ECL_2_DOT_0 = "ECL 2.0"
LICENSE_ECOS = "eCos"
LICENSE_EFL_2_DOT_0 = "EFL 2.0"
LICENSE_ENTESSA = "Entessa"
LICENSE_EPL = "EPL"
LICENSE_ERPL = "ERPL"
LICENSE_EUPL_1_DOT_1 = "EUPL 1.1"
LICENSE_EUROSYM = "Eurosym"
LICENSE_EU_DATAGRID = "EU Datagrid"
LICENSE_FAIR = "Fair"
LICENSE_FBSDDL = "FBSDDL"
LICENSE_FREE_ART = "Free Art"
LICENSE_FTL = "FTL"
LICENSE_GEOGRATIS = "GeoGratis"
LICENSE_GFDL = "GFDL"
LICENSE_GIFTWARE = "Giftware"
LICENSE_GL2PS = "GL2PS"
LICENSE_GLIDE = "Glide"
LICENSE_GNUPLOT = "gnuplot"
LICENSE_GPLV1 = "GPLv1"
LICENSE_GPLV2 = "GPLv2"
LICENSE_GPLV2_OR_ARTISTIC = "GPLv2 or Artistic"
LICENSE_GPLV2_PLUS = "GPLv2+"
LICENSE_GPLV2_PLUS_OR_ARTISTIC = "GPLv2+ or Artistic"
LICENSE_GPLV2_PLUS_WITH_EXCEPTIONS = "GPLv2+ with exceptions"
LICENSE_GPLV2_WITH_EXCEPTIONS = "GPLv2 with exceptions"
LICENSE_GPLV3 = "GPLv3"
LICENSE_GPLV3_PLUS = "GPLv3+"
LICENSE_GPLV3_PLUS_WITH_EXCEPTIONS = "GPLv3+ with exceptions"
LICENSE_GPLV3_WITH_EXCEPTIONS = "GPLv3 with exceptions"
LICENSE_GPL_PLUS = "GPL+"
LICENSE_GPL_PLUS_OR_ARTISTIC = "GPL+ or Artistic"
LICENSE_GPL_PLUS_WITH_EXCEPTIONS = "GPL+ with exceptions"
LICENSE_IBM = "IBM"
LICENSE_IEEE = "IEEE"
LICENSE_IJG = "IJG"
LICENSE_IMAGEMAGICK = "ImageMagick"
LICENSE_IMATIX = "iMatix"
LICENSE_IMLIB2 = "Imlib2"
LICENSE_INTEL_ACPI = "Intel ACPI"
LICENSE_INTERBASE = "Interbase"
LICENSE_IPA = "IPA"
LICENSE_ISC = "ISC"
LICENSE_JABBER = "Jabber"
LICENSE_JASPER = "JasPer"
LICENSE_JPYTHON = "JPython"
LICENSE_KNUTH = "Knuth"
LICENSE_LBNL_BSD = "LBNL BSD"
LICENSE_LDPL = "LDPL"
LICENSE_LGPLV2 = "LGPLv2"
LICENSE_LGPLV2_PLUS = "LGPLv2+"
LICENSE_LGPLV2_PLUS_OR_ARTISTIC = "LGPLv2+ or Artistic"
LICENSE_LGPLV2_PLUS_WITH_EXCEPTIONS = "LGPLv2+ with exceptions"
LICENSE_LGPLV2_WITH_EXCEPTIONS = "LGPLv2 with exceptions"
LICENSE_LGPLV3 = "LGPLv3"
LICENSE_LGPLV3_PLUS = "LGPLv3+"
LICENSE_LGPLV3_PLUS_WITH_EXCEPTIONS = "LGPLv3+ with exceptions"
LICENSE_LGPLV3_WITH_EXCEPTIONS = "LGPLv3 with exceptions"
LICENSE_LIBERATION = "Liberation"
LICENSE_LIBTIFF = "libtiff"
LICENSE_LLGPL = "LLGPL"
LICENSE_LOGICA = "Logica"
LICENSE_LPL = "LPL"
LICENSE_LPPL = "LPPL"
LICENSE_MECAB_IPADIC = "mecab-ipadic"
LICENSE_MIROS = "MirOS"
LICENSE_MIT = "MIT"
LICENSE_MIT_WITH_ADVERTISING = "MIT with advertising"
LICENSE_MOD_MACRO = "mod_macro"
LICENSE_MOTOSOTO = "Motosoto"
LICENSE_MPLUS = "mplus"
LICENSE_MPLV1_DOT_0 = "MPLv1.0"
LICENSE_MPLV1_DOT_1 = "MPLv1.1"
LICENSE_MS_PL = "MS-PL"
LICENSE_MS_RL = "MS-RL"
LICENSE_NAUMEN = "Naumen"
LICENSE_NCSA = "NCSA"
LICENSE_NETCDF = "NetCDF"
LICENSE_NETSCAPE = "Netscape"
LICENSE_NEWMAT = "Newmat"
LICENSE_NGPL = "NGPL"
LICENSE_NOKIA = "Nokia"
LICENSE_NOSL = "NOSL"
LICENSE_NOWEB = "Noweb"
LICENSE_OAL = "OAL"
LICENSE_OFL = "OFL"
LICENSE_OFSFDL = "OFSFDL"
LICENSE_OML = "OML"
LICENSE_OPENLDAP = "OpenLDAP"
LICENSE_OPENPBS = "OpenPBS"
LICENSE_OPENSSL = "OpenSSL"
LICENSE_OREILLY = "OReilly"
LICENSE_OSL_1_DOT_0 = "OSL 1.0"
LICENSE_OSL_1_DOT_1 = "OSL 1.1"
LICENSE_OSL_2_DOT_0 = "OSL 2.0"
LICENSE_OSL_2_DOT_1 = "OSL 2.1"
LICENSE_OSL_3_DOT_0 = "OSL 3.0"
LICENSE_PHORUM = "Phorum"
LICENSE_PHP = "PHP"
LICENSE_PLEXUS = "Plexus"
LICENSE_POSTGRESQL = "PostgreSQL"
LICENSE_PSUTILS = "psutils"
LICENSE_PTFL = "PTFL"
LICENSE_PUBLIC_DOMAIN = "Public Domain"
LICENSE_PUBLIC_USE = "Public Use"
LICENSE_PYTHON = "Python"
LICENSE_QHULL = "Qhull"
LICENSE_QPL = "QPL"
LICENSE_RDISC = "Rdisc"
LICENSE_RICEBSD = "RiceBSD"
LICENSE_ROMIO = "Romio"
LICENSE_RPSL = "RPSL"
LICENSE_RUBY = "Ruby"
LICENSE_SAXPATH = "Saxpath"
LICENSE_SCEA = "SCEA"
LICENSE_SCRIP = "SCRIP"
LICENSE_SENDMAIL = "Sendmail"
LICENSE_SISSL = "SISSL"
LICENSE_SLEEPYCAT = "Sleepycat"
LICENSE_SLIB = "SLIB"
LICENSE_SNIA = "SNIA"
LICENSE_SPL = "SPL"
LICENSE_STIX = "STIX"
LICENSE_TCL = "TCL"
LICENSE_TEEWORLDS = "Teeworlds"
LICENSE_TMATE = "TMate"
LICENSE_TOSL = "TOSL"
LICENSE_TPL = "TPL"
LICENSE_UCD = "UCD"
LICENSE_UNKNOWN = "unknown"
LICENSE_VIM = "Vim"
LICENSE_VNLSL = "VNLSL"
LICENSE_VOSTROM = "VOSTROM"
LICENSE_VSL = "VSL"
LICENSE_W3C = "W3C"
LICENSE_WADALAB = "Wadalab"
LICENSE_WEBMIN = "Webmin"
LICENSE_WTFPL = "WTFPL"
LICENSE_WXWIDGETS = "wxWidgets"
LICENSE_XANO = "XANO"
LICENSE_XEROX = "Xerox"
LICENSE_XINETD = "xinetd"
LICENSE_XSKAT = "XSkat"
LICENSE_YPLV1_DOT_1 = "YPLv1.1"
LICENSE_ZEND = "Zend"
LICENSE_ZLIB = "zlib"
LICENSE_ZLIB_WITH_ACKNOWLEDGEMENT = "zlib with acknowledgement"
LICENSE_ZPLV1_DOT_0 = "ZPLv1.0"
LICENSE_ZPLV2_DOT_0 = "ZPLv2.0"
LICENSE_ZPLV2_DOT_1 = "ZPLv2.1"
MEDIA_TYPE_CD = "cd"
MEDIA_TYPE_DISC = "disc"
MEDIA_TYPE_DVD = "dvd"
MEDIA_TYPE_UNKNOWN = "unknown"
MESSAGE_AUTOREMOVE_IGNORED = "autoremove-ignored"
MESSAGE_BACKEND_ERROR = "backend-error"
MESSAGE_BROKEN_MIRROR = "broken-mirror"
MESSAGE_CACHE_BEING_REBUILT = "cache-being-rebuilt"
MESSAGE_CONFIG_FILES_CHANGED = "config-files-changed"
MESSAGE_CONNECTION_REFUSED = "connection-refused"
MESSAGE_COULD_NOT_FIND_PACKAGE = "could-not-find-package"
MESSAGE_DAEMON_ERROR = "daemon-error"
MESSAGE_NEWER_PACKAGE_EXISTS = "newer-package-exists"
MESSAGE_OTHER_UPDATES_HELD_BACK = "other-updates-held-back"
MESSAGE_PACKAGE_ALREADY_INSTALLED = "package-already-installed"
MESSAGE_PARAMETER_INVALID = "parameter-invalid"
MESSAGE_PRIORITY_INVALID = "priority-invalid"
MESSAGE_REPO_FOR_DEVELOPERS_ONLY = "repo-for-developers-only"
MESSAGE_REPO_METADATA_DOWNLOAD_FAILED = "repo-metadata-download-failed"
MESSAGE_UNKNOWN = "unknown"
MESSAGE_UNTRUSTED_PACKAGE = "untrusted-package"
NETWORK_MOBILE = "mobile"
NETWORK_OFFLINE = "offline"
NETWORK_ONLINE = "online"
NETWORK_UNKNOWN = "unknown"
NETWORK_WIFI = "wifi"
NETWORK_WIRED = "wired"
PROVIDES_ANY = "any"
PROVIDES_CODEC = "codec"
PROVIDES_FONT = "font"
PROVIDES_HARDWARE_DRIVER = "driver"
PROVIDES_MIMETYPE = "mimetype"
PROVIDES_MODALIAS = "modalias"
PROVIDES_POSTSCRIPT_DRIVER = "postscript-driver"
PROVIDES_UNKNOWN = "unknown"
RESTART_APPLICATION = "application"
RESTART_NONE = "none"
RESTART_SECURITY_SESSION = "security-session"
RESTART_SECURITY_SYSTEM = "security-system"
RESTART_SESSION = "session"
RESTART_SYSTEM = "system"
RESTART_UNKNOWN = "unknown"
ROLE_ACCEPT_EULA = "accept-eula"
ROLE_CANCEL = "cancel"
ROLE_DOWNLOAD_PACKAGES = "download-packages"
ROLE_GET_CATEGORIES = "get-categories"
ROLE_GET_DEPENDS = "get-depends"
ROLE_GET_DETAILS = "get-details"
ROLE_GET_DISTRO_UPGRADES = "get-distro-upgrades"
ROLE_GET_FILES = "get-files"
ROLE_GET_OLD_TRANSACTIONS = "get-old-transactions"
ROLE_GET_PACKAGES = "get-packages"
ROLE_GET_REPO_LIST = "get-repo-list"
ROLE_GET_REQUIRES = "get-requires"
ROLE_GET_UPDATES = "get-updates"
ROLE_GET_UPDATE_DETAIL = "get-update-detail"
ROLE_INSTALL_FILES = "install-files"
ROLE_INSTALL_PACKAGES = "install-packages"
ROLE_INSTALL_SIGNATURE = "install-signature"
ROLE_REFRESH_CACHE = "refresh-cache"
ROLE_REMOVE_PACKAGES = "remove-packages"
ROLE_REPO_ENABLE = "repo-enable"
ROLE_REPO_SET_DATA = "repo-set-data"
ROLE_RESOLVE = "resolve"
ROLE_ROLLBACK = "rollback"
ROLE_SEARCH_DETAILS = "search-details"
ROLE_SEARCH_FILE = "search-file"
ROLE_SEARCH_GROUP = "search-group"
ROLE_SEARCH_NAME = "search-name"
ROLE_SIMULATE_INSTALL_FILES = "simulate-install-files"
ROLE_SIMULATE_INSTALL_PACKAGES = "simulate-install-packages"
ROLE_SIMULATE_REMOVE_PACKAGES = "simulate-remove-packages"
ROLE_SIMULATE_UPDATE_PACKAGES = "simulate-update-packages"
ROLE_UNKNOWN = "unknown"
ROLE_UPDATE_PACKAGES = "update-packages"
ROLE_UPDATE_SYSTEM = "update-system"
ROLE_WHAT_PROVIDES = "what-provides"
SIGTYPE_GPG = "gpg"
SIGTYPE_UNKNOWN = "unknown"
STATUS_CANCEL = "cancel"
STATUS_CHECK_EXECUTABLE_FILES = "check-executable-files"
STATUS_CHECK_LIBRARIES = "check-libraries"
STATUS_CLEANUP = "cleanup"
STATUS_COMMIT = "commit"
STATUS_COPY_FILES = "copy-files"
STATUS_DEP_RESOLVE = "dep-resolve"
STATUS_DOWNLOAD = "download"
STATUS_DOWNLOAD_CHANGELOG = "download-changelog"
STATUS_DOWNLOAD_FILELIST = "download-filelist"
STATUS_DOWNLOAD_GROUP = "download-group"
STATUS_DOWNLOAD_PACKAGELIST = "download-packagelist"
STATUS_DOWNLOAD_REPOSITORY = "download-repository"
STATUS_DOWNLOAD_UPDATEINFO = "download-updateinfo"
STATUS_FINISHED = "finished"
STATUS_GENERATE_PACKAGE_LIST = "generate-package-list"
STATUS_INFO = "info"
STATUS_INSTALL = "install"
STATUS_LOADING_CACHE = "loading-cache"
STATUS_OBSOLETE = "obsolete"
STATUS_QUERY = "query"
STATUS_REFRESH_CACHE = "refresh-cache"
STATUS_REMOVE = "remove"
STATUS_REPACKAGING = "repackaging"
STATUS_REQUEST = "request"
STATUS_ROLLBACK = "rollback"
STATUS_RUNNING = "running"
STATUS_SCAN_APPLICATIONS = "scan-applications"
STATUS_SCAN_PROCESS_LIST = "scan-process-list"
STATUS_SETUP = "setup"
STATUS_SIG_CHECK = "sig-check"
STATUS_TEST_COMMIT = "test-commit"
STATUS_UNKNOWN = "unknown"
STATUS_UPDATE = "update"
STATUS_WAIT = "wait"
STATUS_WAITING_FOR_AUTH = "waiting-for-auth"
STATUS_WAITING_FOR_LOCK = "waiting-for-lock"
UPDATE_STATE_STABLE = "stable"
UPDATE_STATE_TESTING = "testing"
UPDATE_STATE_UNKNOWN = "unknown"
UPDATE_STATE_UNSTABLE = "unstable"