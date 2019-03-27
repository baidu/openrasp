#!/bin/sh

# WARNING: This file is created by the Configuration Wizard.
# Any changes to this script may be lost when adding extensions to this configuration.

# --- Start Functions ---

stopAll()
{
	# We separate the stop commands into a function so we are able to use the trap command in Unix (calling a function) to stop these services
	if [ "X${ALREADY_STOPPED}" != "X" ] ; then
		exit
	fi
	# STOP DERBY (only if we started it)
	if [ "${DERBY_FLAG}" = "true" ] ; then
		echo "Stopping Derby server..."
		${WL_HOME}/common/derby/bin/stopNetworkServer.sh  >"${DOMAIN_HOME}/derbyShutdown.log" 2>&1 

		echo "Derby server stopped."
	fi

	ALREADY_STOPPED="true"
}

generateClassList()
{
	JAVA_OPTIONS="${JAVA_OPTIONS} -Xshare:off -XX:+UnlockCommercialFeatures -XX:+IgnoreEmptyClassPaths -XX:DumpLoadedClassList=${APPCDS_CLASS_LIST} -XX:+UseAppCDS"
	export JAVA_OPTIONS
}

useArchive()
{
	JAVA_OPTIONS="${JAVA_OPTIONS} -XX:+UnlockCommercialFeatures -Xshare:auto -XX:+UseAppCDS -XX:+IgnoreEmptyClassPaths -XX:SharedArchiveFile=${APPCDS_ARCHIVE} -showversion"
	export JAVA_OPTIONS
	USING_SHOWVERSION="true"
}

# --- End Functions ---

# *************************************************************************
# This script is used to start WebLogic Server for this domain.
# 
# To create your own start script for your domain, you can initialize the
# environment by calling @USERDOMAINHOME/setDomainEnv.
# 
# setDomainEnv initializes or calls commEnv to initialize the following variables:
# 
# BEA_HOME       - The BEA home directory of your WebLogic installation.
# JAVA_HOME      - Location of the version of Java used to start WebLogic
#                  Server.
# JAVA_VENDOR    - Vendor of the JVM (i.e. BEA, HP, IBM, Sun, etc.)
# PATH           - JDK and WebLogic directories are added to system path.
# WEBLOGIC_CLASSPATH
#                - Classpath needed to start WebLogic Server.
# PATCH_CLASSPATH - Classpath used for patches
# PATCH_LIBPATH  - Library path used for patches
# PATCH_PATH     - Path used for patches
# WEBLOGIC_EXTENSION_DIRS - Extension dirs for WebLogic classpath patch
# JAVA_VM        - The java arg specifying the VM to run.  (i.e.
#                - server, -hotspot, etc.)
# USER_MEM_ARGS  - The variable to override the standard memory arguments
#                  passed to java.
# PRODUCTION_MODE - The variable that determines whether Weblogic Server is started in production mode.
# DERBY_HOME - Derby home directory.
# DERBY_CLASSPATH
#                - Classpath needed to start Derby.
# 
# Other variables used in this script include:
# SERVER_NAME    - Name of the weblogic server.
# JAVA_OPTIONS   - Java command-line options for running the server. (These
#                  will be tagged on to the end of the JAVA_VM and
#                  MEM_ARGS)
# PROXY_SETTINGS - These are tagged on to the end of the JAVA_OPTIONS. This variable is deprecated and should not
#                  be used. Instead use JAVA_OPTIONS
# 
# For additional information, refer to "Administering Server Startup and Shutdown for Oracle WebLogic Server"
# *************************************************************************

umask 027


mypwd="$(pwd)"
SCRIPT_PATH="$0"
case ${SCRIPT_PATH} in /*) SCRIPTPATH=`dirname "${SCRIPT_PATH}"`;; *) SCRIPTPATH=`dirname "${mypwd}/${SCRIPT_PATH}"`;; esac 


# Call setDomainEnv here.

DOMAIN_HOME="/home/weblogic/weblogicInstall/wls12213/user_projects/domains/base_domain"

. ${DOMAIN_HOME}/bin/setDomainEnv.sh $*

JAVA_OPTIONS="${JAVA_OPTIONS} -Djava.security.egd=file:/dev/urandom"

SAVE_JAVA_OPTIONS="${JAVA_OPTIONS}"

SAVE_CLASSPATH="${CLASSPATH}"

TMP_UPDATE_SCRIPT="/tmp/Update.sh"
export TMP_UPDATE_SCRIPT


# Start Derby

DERBY_DEBUG_LEVEL="0"

if [ "${DERBY_FLAG}" = "true" ] ; then
	${WL_HOME}/common/derby/bin/startNetworkServer.sh  >"${DOMAIN_HOME}/derby.log" 2>&1 

fi

JAVA_OPTIONS="${SAVE_JAVA_OPTIONS}"
### BEGIN OPENRASP - DO NOT MODIFY ###
JAVA_OPTIONS="-javaagent:${DOMAIN_HOME}/rasp/rasp.jar ${JAVA_OPTIONS}"
JAVA_OPTIONS="-javaagent:${DOMAIN_HOME}/jacocoagent.jar=destfile=${DOMAIN_HOME}/weblogic.exec,inclbootstrapclasses=true,append=true -Xverify:none ${JAVA_OPTIONS}"
### END OPENRASP ###

# In order to use resource consumption management policies or to get partition's resource consumption metrics, uncomment the following JAVA_OPTIONS

#JAVA_OPTIONS="-XX:+UnlockCommercialFeatures -XX:+ResourceManagement -XX:+UseG1GC ${SAVE_JAVA_OPTIONS}"

SAVE_JAVA_OPTIONS=""

CLASSPATH="${SAVE_CLASSPATH}"

SAVE_CLASSPATH=""

trap 'stopAll' 1 2 3 15


if [ "${PRODUCTION_MODE}" = "true" ] ; then
	WLS_DISPLAY_MODE="Production"
else
	WLS_DISPLAY_MODE="Development"
fi

if [ "${WLS_USER}" != "" ] ; then
	JAVA_OPTIONS="${JAVA_OPTIONS} -Dweblogic.management.username=${WLS_USER}"
fi

if [ "${WLS_PW}" != "" ] ; then
	JAVA_OPTIONS="${JAVA_OPTIONS} -Dweblogic.management.password=${WLS_PW}"
fi

if [ "${MEDREC_WEBLOGIC_CLASSPATH}" != "" ] ; then
	if [ "${CLASSPATH}" != "" ] ; then
		CLASSPATH="${CLASSPATH}${CLASSPATHSEP}${MEDREC_WEBLOGIC_CLASSPATH}"
	else
		CLASSPATH="${MEDREC_WEBLOGIC_CLASSPATH}"
	fi
fi

if [ "${GENERATE_CLASS_LIST}" = "true" ] ; then
	generateClassList
fi

if [ "${USE_ARCHIVE}" = "true" ] ; then
	useArchive
fi

echo "."

echo "."

echo "JAVA Memory arguments: ${MEM_ARGS}"

echo "."

echo "CLASSPATH=${CLASSPATH}"

echo "."

echo "PATH=${PATH}"

echo "."

echo "***************************************************"

echo "*  To start WebLogic Server, use a username and   *"

echo "*  password assigned to an admin-level user.  For *"

echo "*  server administration, use the WebLogic Server *"

echo "*  console at http://hostname:port/console        *"

echo "***************************************************"

# START WEBLOGIC

if [ "${USE_JVM_SYSTEM_LOADER}" != "true" ] ; then
	LAUNCH_ARGS="-cp ${WL_HOME}/server/lib/weblogic-launcher.jar -Dlaunch.use.env.classpath=true"
	export LAUNCH_ARGS
fi

if [ "${USING_SHOWVERSION}" = "true" ] ; then
	echo "starting weblogic with Java version:"
	${JAVA_HOME}/bin/java ${JAVA_VM} -version
fi

if [ "${WLS_REDIRECT_LOG}" = "" ] ; then
	echo "Starting WLS with line:"
	echo "${JAVA_HOME}/bin/java ${JAVA_VM} ${MEM_ARGS} ${LAUNCH_ARGS} -Dweblogic.Name=${SERVER_NAME} -Djava.security.policy=${WLS_POLICY_FILE} ${JAVA_OPTIONS} ${PROXY_SETTINGS} ${SERVER_CLASS}"
	${JAVA_HOME}/bin/java ${JAVA_VM} ${MEM_ARGS} ${LAUNCH_ARGS} -Dweblogic.Name=${SERVER_NAME} -Djava.security.policy=${WLS_POLICY_FILE} ${JAVA_OPTIONS} ${PROXY_SETTINGS} ${SERVER_CLASS}
else
	echo "Redirecting output from WLS window to ${WLS_REDIRECT_LOG}"
	${JAVA_HOME}/bin/java ${JAVA_VM} ${MEM_ARGS} ${LAUNCH_ARGS} -Dweblogic.Name=${SERVER_NAME} -Djava.security.policy=${WLS_POLICY_FILE} ${JAVA_OPTIONS} ${PROXY_SETTINGS} ${SERVER_CLASS}  >"${WLS_REDIRECT_LOG}" 2>&1 
fi

shutDownStatus=$?


stopAll

popd

if [ -f ${TMP_UPDATE_SCRIPT} ];then fileExists=true;else fileExists=false;fi


if [ "${shutDownStatus}" = "86" ] ; then
	if [ "${fileExists}" = "true" ] ; then
		echo "Calling ${TMP_UPDATE_SCRIPT}"
		(cd /tmp;${TMP_UPDATE_SCRIPT})
		ustatus=$?


		# restoring the original env before unsetting JAVA_HOME
		# in order to unset any JAVA_HOME that was passed in from domainEnv
		if [ "${ustatus}" = "42" ] ; then
			JAVA_HOME=""
		fi
	else
		echo "ERROR! ${TMP_UPDATE_SCRIPT} did not exist"
	fi
	# Call the same script path that was supplied in order to restart ourselves
	# restoreOrigEnv will go here
	exec ${SCRIPTPATH}/startWebLogic.sh


else
	if [ "${shutDownStatus}" = "88" ] ; then
		# restoreOrigEnv will go here
		exec ${SCRIPTPATH}/startWebLogic.sh


	fi
fi

# Exit this script only if we have been told to exit.

if [ "${doExitFlag}" = "true" ] ; then
	exit
fi
