#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "AuthManager.h"

LoginDialog::LoginDialog(AuthManager* auth, QWidget* parent) : QDialog(parent), m_auth(auth) {
    setWindowTitle("Sign in");
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel("Email:"));
    m_email = new QLineEdit(this);
    lay->addWidget(m_email);
    lay->addWidget(new QLabel("Password:"));
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);
    lay->addWidget(m_password);

    m_signInBtn = new QPushButton("Sign In", this);
    m_signUpBtn = new QPushButton("Sign Up", this);
    lay->addWidget(m_signInBtn);
    lay->addWidget(m_signUpBtn);

    connect(m_signInBtn, &QPushButton::clicked, this, &LoginDialog::onSignIn);
    connect(m_signUpBtn, &QPushButton::clicked, this, &LoginDialog::onSignUp);
    connect(m_auth, &AuthManager::signedIn, this, &LoginDialog::onSignedIn);
    connect(m_auth, &AuthManager::authFailed, this, &LoginDialog::onAuthFailed);
}

void LoginDialog::onSignIn() {
    m_signInBtn->setEnabled(false);
    m_auth->signIn(m_email->text(), m_password->text());
}

void LoginDialog::onSignUp() {
    m_signUpBtn->setEnabled(false);
    m_auth->signUp(m_email->text(), m_password->text());
}

void LoginDialog::onSignedIn() {
    accept();
}

void LoginDialog::onAuthFailed(const QString& error) {
    m_signInBtn->setEnabled(true);
    m_signUpBtn->setEnabled(true);
    // show message simply
    auto *d = new QDialog(this);
    d->setWindowTitle("Auth error");
    auto *l = new QVBoxLayout(d);
    l->addWidget(new QLabel(error));
    d->exec();
}
