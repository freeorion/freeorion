#!/bin/bash

{ echo "${DEPLOY_SSH_PASSWORD}" | gpg --batch --yes --skip-verify --passphrase-fd 0 --output .github/id_ed25519 -d .github/id_ed25519.gpg ; } || exit 1
chmod 600 .github/id_ed25519 || exit 1

{ echo "${DEPLOY_SSH_PASSWORD}" | gpg --batch --yes --skip-verify --passphrase-fd 0 --output .github/secring.auto -d .github/secring.auto.gpg ; } || exit 1
chmod 600 .github/secring.auto || exit 1

gpg --import .github/secring.auto
gpg --import .github/pubring.auto

mkdir -p $HOME/.ssh/
chmod 700 $HOME/.ssh/
ssh-keyscan frs.sourceforge.net >> $HOME/.ssh/known_hosts
echo "StrictHostKeyChecking no" >> $HOME/.ssh/config
echo "IdentityFile $(pwd)/.github/id_ed25519" >> $HOME/.ssh/config
echo "PubkeyAuthentication yes" >> $HOME/.ssh/config
echo "IdentitiesOnly yes" >> $HOME/.ssh/config
echo "BatchMode yes" >> $HOME/.ssh/config
echo "ConnectionAttempts 2" >> $HOME/.ssh/config
echo "PasswordAuthentication no" >> $HOME/.ssh/config
echo "PubkeyAcceptedKeyTypes +ssh-ed25519" >> $HOME/.ssh/config

echo Success!

