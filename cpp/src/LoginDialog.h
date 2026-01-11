#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;
class AuthManager;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(AuthManager* auth, QWidget* parent = nullptr);

private slots:
    void onSignIn();
    void onSignUp();
    void onSignedIn();
    void onAuthFailed(const QString& error);

private:
    AuthManager* m_auth;
    QLineEdit* m_email;
    QLineEdit* m_password;
    QPushButton* m_signInBtn;
    QPushButton* m_signUpBtn;
};