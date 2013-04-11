#!/bin/sh

USERNAME=$(id -un)
PRIMARYGROUP=$(id -gn)

echo "Building buildroot will attempt to overwrite the Alix Toolchain in /opt."
echo "This might fail unless you are logged in with an account in the same group."
echo
echo "You might try an 'sudo chown -R $USERNAME.$PRIMARYGROUP /opt/alix_toolchain' before continuing...."
echo
echo "Do you really want to try? Type YES."
read answer
if [ "$answer" = "YES" ]
then 
    exit 0
fi
exit 1
