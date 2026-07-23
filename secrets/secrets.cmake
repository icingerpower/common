# Shared secrets module: OS keychain (Secret Service via QtDBus) + encrypted
# passphrase bundle (OpenSSL). Including targets MUST also link:
#     Qt6::DBus  and  OpenSSL::Crypto
# and find_package(OpenSSL REQUIRED) / Qt6 COMPONENTS DBus.
set(SECRETS_FILES
    ${CMAKE_CURRENT_LIST_DIR}/SecretStore.h
    ${CMAKE_CURRENT_LIST_DIR}/SecretStore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SecretBundle.h
    ${CMAKE_CURRENT_LIST_DIR}/SecretBundle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CredentialManager.h
    ${CMAKE_CURRENT_LIST_DIR}/CredentialManager.cpp
)
