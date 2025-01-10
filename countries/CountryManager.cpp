#include <QtCore/qdatetime.h>
#include <QtCore/qset.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qsharedpointer.h>

#include "CountryManager.h"

QString CountryManager::CZECH = QObject::tr("République-Tchèque");
QString CountryManager::ITALY = QObject::tr("Italie");
QString CountryManager::SPAIN = QObject::tr("Espagne");
QString CountryManager::FRANCE = QObject::tr("France");
QString CountryManager::POLAND = QObject::tr("Pologne");
QString CountryManager::SWEDEN = QObject::tr("Suède");
QString CountryManager::GERMANY = QObject::tr("Allemagne");
QString CountryManager::NETHERLAND = QObject::tr("Pays-bas");
QString CountryManager::BELGIUM = QObject::tr("Belgique");
QString CountryManager::AUSTRIA = QObject::tr("Autriche");
QString CountryManager::BULGARIA = QObject::tr("Bulgarie");
QString CountryManager::CHYPRE = QObject::tr("Chypre");
QString CountryManager::CROATIA = QObject::tr("Croatie");
QString CountryManager::DENMARK = QObject::tr("Danemark");
QString CountryManager::ESTONIA = QObject::tr("Estonie");
QString CountryManager::FINLAND = QObject::tr("Finlande");
QString CountryManager::GREECE = QObject::tr("Grèce");
QString CountryManager::HONGRY = QObject::tr("Hongrie");
QString CountryManager::IRELAND = QObject::tr("Irelande");
QString CountryManager::IRELAND_NORTHEN = QObject::tr("Irelande du nord");
QString CountryManager::LATVIA = QObject::tr("Lettonie");
QString CountryManager::LUXEMBOURG = QObject::tr("Luxembourg");
QString CountryManager::MALTA = QObject::tr("Malte");
QString CountryManager::PORTUGAL = QObject::tr("Portugale");
QString CountryManager::ROMANIA = QObject::tr("Roumanie");
QString CountryManager::SLOVAKIA = QObject::tr("Slovaquie");
QString CountryManager::SLOVENIA = QObject::tr("Slovénie");
QString CountryManager::LITHUANIA = QObject::tr("Lituanie");
QString CountryManager::UK = QObject::tr("Grande-bretagne");
QString CountryManager::EU = QObject::tr("UE");
QString CountryManager::CHINA = QObject::tr("Chine");
//----------------------------------------------------------
CountryManager::CountryManager()
{
}
//----------------------------------------------------------
CountryManager *CountryManager::instance()
{
    static CountryManager instance;
    return &instance;
}
//----------------------------------------------------------
const QStringList *CountryManager::countryCodes()
{
    static QStringList countries
            = []() -> QStringList {
            QStringList allCountries
            = {"FR","MC","DE","IT","ES","PL","CZ","NL","SE"
               ,"BE","BG","DK","EE","GR","GB-NIR","IE","EL","HR","CY"
               ,"LV","LT","LU","HU","MT","AT","PT","RO","SI"
               ,"SK","FI","US","CA","CN","GB","AU","CH"};
            std::sort(allCountries.begin(), allCountries.end());
            return allCountries;
}();
    return &countries;
}
//----------------------------------------------------------
const QStringList *CountryManager::countryNames()
{
    static QStringList countryNames
            = {FRANCE
               , UK
               , GERMANY
               , ITALY
               , SPAIN
               , POLAND
               , CZECH
               , NETHERLAND
               , SWEDEN
               , BELGIUM
               , BULGARIA
               , DENMARK
               , ESTONIA
               , IRELAND
               , IRELAND_NORTHEN
               , GREECE
               , CROATIA
               , CHYPRE
               , LATVIA
               , LITHUANIA
               , LUXEMBOURG
               , MALTA
               , PORTUGAL
               , ROMANIA
               , SLOVENIA
               , SLOVAKIA
               , FINLAND
               , QObject::tr("Hongrie")
               , AUSTRIA
               , QObject::tr("Guadeloupe")
               , QObject::tr("Monaco")
               , QObject::tr("Icelande")
               , QObject::tr("Canada")
               , QObject::tr("Émirats arabes unis")
               , QObject::tr("États-Unis")
               , QObject::tr("Réunion")
               , QObject::tr("Martinique")
               , QObject::tr("Guyane")
               , QObject::tr("Guadeloupe")
               , QObject::tr("Mayotte")
               , QObject::tr("Suisse")
               , CHINA
               , QObject::tr("Afghanistan")
               , QObject::tr("Albanie")
               , QObject::tr("Algérie")
               , QObject::tr("Samoa américaines")
               , QObject::tr("Andorre")
               , QObject::tr("Angola")
               , QObject::tr("Anguilla")
               , QObject::tr("Antigua-et-Barbuda")
               , QObject::tr("Argentine")
               , QObject::tr("Arménie")
               , QObject::tr("Aruba")
               , QObject::tr("Australie")
               , QObject::tr("Autriche")
               , QObject::tr("Azerbaïdjan")
               , QObject::tr("Bahamas (les)")
               , QObject::tr("Bahreïn")
               , QObject::tr("Bengladesh")
               , QObject::tr("Barbade")
               , QObject::tr("Biélorussie")
               , QObject::tr("Belgique"), QObject::tr("Bélize"), QObject::tr("Bénin"), QObject::tr("Bermudes")
               , QObject::tr("Bhoutan"), QObject::tr("Bolivie"), QObject::tr("Bonaire, Saint Eustache et Saba")
               , QObject::tr("Bosnie Herzégovine"), QObject::tr("Bostwana"), QObject::tr("Île Bouvet")
               , QObject::tr("Brésil"), QObject::tr("Territoire britannique de l'océan Indien"), QObject::tr("Brunei Darussalam")
               , QObject::tr("Bulgarie"), QObject::tr("Burkina Faso"), QObject::tr("Burundi")
               , QObject::tr("Cap-Vert"), QObject::tr("Cambodge"), QObject::tr("Cameroun"), QObject::tr("Canada")
               , QObject::tr("Îles Caïmans"), QObject::tr("République centrafricaine"), QObject::tr("Tchad")
               , QObject::tr("Chili"), CHINA, QObject::tr("L'île de noël"), QObject::tr("Îles Cocos")
               , QObject::tr("Colombie"), QObject::tr("Comores"), QObject::tr("Congo (République démocratique)")
               , QObject::tr("Congo"), QObject::tr("Îles Cook (les)"), QObject::tr("Costa Rica")
               , QObject::tr("Croatie"), QObject::tr("Cuba"), QObject::tr("Curacao"), QObject::tr("Chypre")
               ,QObject::tr("Côte d'Ivoire"), QObject::tr("Danemark")
               , QObject::tr("Djibouti"), QObject::tr("Dominique"), QObject::tr("République dominicaine")
               , QObject::tr("Equateur"), QObject::tr("Egypte"), QObject::tr("Le Salvador")
               , QObject::tr("Guinée Équatoriale"), QObject::tr("Érythrée"), QObject::tr("Estonie")
               , QObject::tr("Eswatini"), QObject::tr("Ethiopie"), QObject::tr("Îles Falkland")
               , QObject::tr("Îles Féroé"), QObject::tr("Fidji"), QObject::tr("Finlande")
               , QObject::tr("France"), QObject::tr("Guyane Française"), QObject::tr("Polynésie française")
               , QObject::tr("Terres australes françaises"), QObject::tr("Gabon"), QObject::tr("Gambie (la)")
               , QObject::tr("Géorgie"), QObject::tr("Allemagne"), QObject::tr("Ghana")
               , QObject::tr("Gibraltar"), QObject::tr("Grèce"), QObject::tr("Grenade")
               , QObject::tr("Guadeloupe"), QObject::tr("Guam"), QObject::tr("Guatemala"), QObject::tr("Guernesey")
               , QObject::tr("Guinée")
               , QObject::tr("Guinée-Bissau")
               , QObject::tr("Guyane")
               , QObject::tr("Haïti")
               , QObject::tr("Île Heard et îles McDonald")
               , QObject::tr("Saint-Siège")
               , QObject::tr("Honduras")
               , QObject::tr("Hong Kong")
               , QObject::tr("Hongrie")
               , QObject::tr("Islande")
               , QObject::tr("Inde")
               , QObject::tr("Indonésie")
               , QObject::tr("Iran")
               , QObject::tr("Irak")
               , QObject::tr("île de Man")
               , QObject::tr("Israël")
               , QObject::tr("Italie")
               , QObject::tr("Jamaïque")
               , QObject::tr("Japon")
               , QObject::tr("Jersey")
               , QObject::tr("Jordan")
               , QObject::tr("Kazakhstan")
               , QObject::tr("Kenya")
               , QObject::tr("Kiribati")
               , QObject::tr("Corée du nord")
               , QObject::tr("Corée du sud")
               , QObject::tr("Koweit")
               , QObject::tr("Kirghizistan")
               , QObject::tr("Laos")
               , QObject::tr("Lettonie")
               , QObject::tr("Liban"), QObject::tr("Lesotho"), QObject::tr("Libéria"), QObject::tr("Libye")

               , QObject::tr("Liechtenstein"), QObject::tr("Lituanie"), QObject::tr("Luxembourg"), QObject::tr("Macao")
               , QObject::tr("Madagascar"), QObject::tr("Malawi"), QObject::tr("Malaisie"), QObject::tr("Maldives")
               , QObject::tr("Mali"), QObject::tr("Malte"), QObject::tr("Îles Marshall (les)"), QObject::tr("Martinique")
               , QObject::tr("Mauritanie"), QObject::tr("Maurice"), QObject::tr("Mayotte"), QObject::tr("Mexique")
               , QObject::tr("Micronésie"), QObject::tr("Moldavie"), QObject::tr("Monaco"), QObject::tr("Mongolie")
               , QObject::tr("Monténégro"), QObject::tr("Montserrat"), QObject::tr("Maroc"), QObject::tr("Mozambique")
               , QObject::tr("Birmanie"), QObject::tr("Namibie"), QObject::tr("Nauru"), QObject::tr("Népal")
               , QObject::tr("Nouvelle Calédonie"), QObject::tr("Nouvelle-Zélande")
               , QObject::tr("Nicaragua"), QObject::tr("Niger (le)"), QObject::tr("Nigeria"), QObject::tr("Niué")
               , QObject::tr("l'ile de Norfolk"), QObject::tr("Îles Mariannes du Nord"), QObject::tr("Norvège")
               , QObject::tr("Oman")
               , QObject::tr("Pakistan")
               , QObject::tr("Palaos")
               , QObject::tr("Palestine, État de")
               , QObject::tr("Panama")
               , QObject::tr("Papouasie Nouvelle Guinée")
               , QObject::tr("Paraguay")
               , QObject::tr("Pérou")
               , QObject::tr("Philippines (les)")
               , QObject::tr("Pitcairn")
               , QObject::tr("Pologne")
               , QObject::tr("Porto Rico")
               , QObject::tr("Qatar")
               , QObject::tr("République de Macédoine du Nord")
               , QObject::tr("Roumanie")
               , QObject::tr("Fédération de Russie (la)")
               , QObject::tr("Rwanda")
               , QObject::tr("Réunion")
               , QObject::tr("Saint Barthélemy")
               , QObject::tr("Sainte-Hélène, Ascension et Tristan da Cunha")
               , QObject::tr("Saint-Christophe-et-Niévès")
               , QObject::tr("Sainte-Lucie")
               , QObject::tr("Saint Martin (partie française)")
               , QObject::tr("Saint-Pierre-et-Miquelon")
               , QObject::tr("Saint-Vincent-et-les-Grenadines")
               , QObject::tr("Samoa")
               , QObject::tr("Saint Marin")
               , QObject::tr("Sao Tomé et Principe")
               , QObject::tr("Arabie Saoudite")
               , QObject::tr("Sénégal")
               , QObject::tr("Serbie")
               , QObject::tr("les Seychelles")
               , QObject::tr("Sierra Leone")
               , QObject::tr("Singapour")
               , QObject::tr("Sint Maarten")
               , QObject::tr("Slovaquie")
               , QObject::tr("Slovénie")
               , QObject::tr("îles Salomon")
               , QObject::tr("Somalie")
               , QObject::tr("Afrique du Sud")
               , QObject::tr("Soudan du sud")
               , QObject::tr("Espagne")
               , QObject::tr("Sri Lanka")
               , QObject::tr("Soudan")
               , QObject::tr("Suriname")
               , QObject::tr("Svalbard et Jan Mayen")
               , QObject::tr("Suède")
               , QObject::tr("Suisse")
               , QObject::tr("République arabe syrienne")
               , QObject::tr("Taiwan")
               , QObject::tr("Tadjikistan")
               , QObject::tr("Tanzanie, République-Unie de")
               , QObject::tr("Thaïlande")
               , QObject::tr("Timor oriental")
               , QObject::tr("Aller")
               , QObject::tr("Tokélaou")
               , QObject::tr("Tonga")
               , QObject::tr("Trinité-et-Tobago")
               , QObject::tr("Tunisie")
               , QObject::tr("Turquie")
               , QObject::tr("Turkménistan")
               , QObject::tr("Îles Turques et Caïques")
               , QObject::tr("Tuvalu")
               , QObject::tr("Ouganda")
               , QObject::tr("Ukraine")
               , QObject::tr("Emirats Arabes Unis")
               //, QObject::tr("Royaume-Uni de Grande-Bretagne et d'Irlande du Nord")
               , QObject::tr("Îles mineures éloignées des États-Unis")
               , QObject::tr("États-Unis")
               , QObject::tr("Uruguay")
               , QObject::tr("Ouzbékistan")
               , QObject::tr("Vanuatu")
               , QObject::tr("Venezuela")
               , QObject::tr("Vietnam")
               , QObject::tr("Îles Vierges britanniques")
               , QObject::tr("Îles Vierges des États-Unis")
               , QObject::tr("Wallis et Futuna")
               , QObject::tr("Sahara occidental")
               , QObject::tr("Yémen")
               , QObject::tr("Zambie")
               , QObject::tr("Zimbabwe")
               , QObject::tr("Iles Aland")
               , QObject::tr("Afghanistan")
               , QObject::tr("Albanie")
               , QObject::tr("Algérie")
               , QObject::tr("Samoa américaines")
               , QObject::tr("Andorre")
               , QObject::tr("Angola")
               , QObject::tr("Anguilla")
               , QObject::tr("Antigua-et-Barbuda")
               , QObject::tr("Argentine")
               , QObject::tr("Arménie")
               , QObject::tr("Aruba")
               , QObject::tr("Australie")
               , QObject::tr("Azerbaïdjan")
               , QObject::tr("Bahamas (les)")
               , QObject::tr("Bahreïn")
               , QObject::tr("Bengladesh")
               , QObject::tr("Barbade")
               , QObject::tr("Biélorussie")
               , QObject::tr("Bélize")
               , QObject::tr("Bénin")
               , QObject::tr("Bermudes")
               , QObject::tr("Bhoutan")
               , QObject::tr("Bolivie")
               , QObject::tr("Bonaire, Saint Eustache et Saba")
               , QObject::tr("Bosnie Herzégovine")
               , QObject::tr("Bostwana")
               , QObject::tr("Île Bouvet")
               , QObject::tr("Brésil")
               , QObject::tr("Territoire britannique de l'océan Indien")
               , QObject::tr("Brunei Darussalam")
               , QObject::tr("Burkina Faso")
               , QObject::tr("Burundi")
               , QObject::tr("Cap-Vert")
               , QObject::tr("Cambodge")
               , QObject::tr("Cameroun")
               , QObject::tr("Îles Caïmans")
               , QObject::tr("République centrafricaine")
               , QObject::tr("Tchad")
               , QObject::tr("Chili")
               , QObject::tr("L'île de noël")
               , QObject::tr("Îles Cocos")
               , QObject::tr("Colombie")
               , QObject::tr("Comores")
               , QObject::tr("Congo (République démocratique)")
               , QObject::tr("Congo")
               , QObject::tr("Îles Cook (les)")
               , QObject::tr("Costa Rica")
               , QObject::tr("Cuba")
               , QObject::tr("Curacao")
               , QObject::tr("Côte d'Ivoire")
               , QObject::tr("Djibouti")
               , QObject::tr("Dominique")
               , QObject::tr("République dominicaine")
               , QObject::tr("Equateur")
               , QObject::tr("Egypte")
               , QObject::tr("Le Salvador")
               , QObject::tr("Guinée Équatoriale")
               , QObject::tr("Érythrée")
               , QObject::tr("Eswatini")
               , QObject::tr("Ethiopie")
               , QObject::tr("Îles Falkland")
               , QObject::tr("Îles Féroé")
               , QObject::tr("Fidji")
               , QObject::tr("Polynésie française")
               , QObject::tr("Terres australes françaises")
               , QObject::tr("Gabon")
               , QObject::tr("Gambie (la)")
               , QObject::tr("Géorgie")
               , QObject::tr("Ghana")
               , QObject::tr("Gibraltar")
               , QObject::tr("Grenade")
               , QObject::tr("Guam")
               , QObject::tr("Guatemala")
               , QObject::tr("Guernesey")
               , QObject::tr("Guinée")
               , QObject::tr("Guinée-Bissau")
               , QObject::tr("Guyane")
               , QObject::tr("Haïti")
               , QObject::tr("Île Heard et îles McDonald")
               , QObject::tr("Saint-Siège")
               , QObject::tr("Honduras")
               , QObject::tr("Hong Kong")
               , QObject::tr("Inde")
               , QObject::tr("Indonésie")
               , QObject::tr("Iran")
               , QObject::tr("Irak")
               , QObject::tr("île de Man")
               , QObject::tr("Israël")
               , QObject::tr("Jamaïque")
               , QObject::tr("Japon")
               , QObject::tr("Jersey")
               , QObject::tr("Jordan")
               , QObject::tr("Kazakhstan")
               , QObject::tr("Kenya")
               , QObject::tr("Kiribati")
               , QObject::tr("Corée du nord")
               , QObject::tr("Corée du sud")
               , QObject::tr("Koweit")
               , QObject::tr("Kirghizistan")
               , QObject::tr("Laos")
               , QObject::tr("Liban")
               , QObject::tr("Lesotho")
               , QObject::tr("Libéria")
               , QObject::tr("Libye")
               , QObject::tr("Liechtenstein")
               , QObject::tr("Macao")
               , QObject::tr("Madagascar")
               , QObject::tr("Malawi")
               , QObject::tr("Malaisie")
               , QObject::tr("Maldives")
               , QObject::tr("Mali")
               , QObject::tr("Îles Marshall (les)")
               , QObject::tr("Mauritanie")
               , QObject::tr("Maurice")
               , QObject::tr("Mexique")
               , QObject::tr("Micronésie")
               , QObject::tr("Moldavie")
               , QObject::tr("Mongolie")
               , QObject::tr("Monténégro")
               , QObject::tr("Montserrat")
               , QObject::tr("Maroc")
               , QObject::tr("Mozambique")
               , QObject::tr("Birmanie")
               , QObject::tr("Namibie")
               , QObject::tr("Nauru")
               , QObject::tr("Népal")
               , QObject::tr("Nouvelle Calédonie")
               , QObject::tr("Nouvelle-Zélande")
               , QObject::tr("Nicaragua")
               , QObject::tr("Niger (le)")
               , QObject::tr("Nigeria")
               , QObject::tr("Niué")
               , QObject::tr("l'ile de Norfolk")
               , QObject::tr("Îles Mariannes du Nord")
               , QObject::tr("Norvège")
               , QObject::tr("Oman")
               , QObject::tr("Pakistan")
               , QObject::tr("Palaos")
               , QObject::tr("Palestine, État de")
               , QObject::tr("Panama")
               , QObject::tr("Papouasie Nouvelle Guinée")
               , QObject::tr("Paraguay")
               , QObject::tr("Pérou")
               , QObject::tr("Philippines (les)")
               , QObject::tr("Pitcairn")
               , QObject::tr("Porto Rico")
               , QObject::tr("Qatar")
               , QObject::tr("République de Macédoine du Nord")
               , QObject::tr("Fédération de Russie (la)")
               , QObject::tr("Rwanda")
               , QObject::tr("Saint Barthélemy")
               , QObject::tr("Sainte-Hélène, Ascension et Tristan da Cunha")
               , QObject::tr("Saint-Christophe-et-Niévès")
               , QObject::tr("Sainte-Lucie")
               , QObject::tr("Saint Martin (partie française)")
               , QObject::tr("Saint-Pierre-et-Miquelon")
               , QObject::tr("Saint-Vincent-et-les-Grenadines")
               , QObject::tr("Samoa")
               , QObject::tr("Saint Marin")
               , QObject::tr("Sao Tomé et Principe")
               , QObject::tr("Arabie Saoudite")
               , QObject::tr("Sénégal")
               , QObject::tr("Serbie")
               , QObject::tr("les Seychelles")
               , QObject::tr("Sierra Leone")
               , QObject::tr("Singapour")
               , QObject::tr("Sint Maarten")
               , QObject::tr("îles Salomon")
               , QObject::tr("Somalie")
               , QObject::tr("Afrique du Sud")
               , QObject::tr("Soudan du sud")
               , QObject::tr("Sri Lanka")
               , QObject::tr("Soudan")
               , QObject::tr("Suriname")
               , QObject::tr("Svalbard et Jan Mayen")
               , QObject::tr("République arabe syrienne")
               , QObject::tr("Taiwan")
               , QObject::tr("Tadjikistan")
               , QObject::tr("Tanzanie, République-Unie de")
               , QObject::tr("Thaïlande")
               , QObject::tr("Timor oriental")
               , QObject::tr("Aller")
               , QObject::tr("Tokélaou")
               , QObject::tr("Tonga")
               , QObject::tr("Trinité-et-Tobago")
               , QObject::tr("Tunisie")
               , QObject::tr("Turquie")
               , QObject::tr("Turkménistan")
               , QObject::tr("Îles Turques et Caïques")
               , QObject::tr("Tuvalu")
               , QObject::tr("Ouganda")
               , QObject::tr("Ukraine")
               , QObject::tr("Îles mineures éloignées des États-Unis")
               , QObject::tr("Uruguay")
               , QObject::tr("Ouzbékistan")
               , QObject::tr("Vanuatu")
               , QObject::tr("Venezuela")
               , QObject::tr("Vietnam")
               , QObject::tr("Îles Vierges britanniques")
               , QObject::tr("Îles Vierges des États-Unis")
               , QObject::tr("Wallis et Futuna")
               , QObject::tr("Sahara occidental")
               , QObject::tr("Yémen")
               , QObject::tr("Zambie")
               , QObject::tr("Zimbabwe")
               , QObject::tr("Iles Aland")
              };
    return &countryNames;
}
//----------------------------------------------------------
const QStringList *CountryManager::countryNamesSorted()
{
    static QStringList names
            = [](){
        QStringList _names = *CountryManager::countryNames();
        std::sort(_names.begin(), _names.end());
        return _names;
    }();
    return &names;
}
//----------------------------------------------------------
const QStringList *CountryManager::countriesFbaCentersUE()
{
    static QStringList countryNames ={
        FRANCE
        , GERMANY
        , ITALY
        , SPAIN
        , SWEDEN
        , POLAND
        , CZECH
    };
    return &countryNames;
}
//----------------------------------------------------------
const QStringList *CountryManager::countriesCodeUE()
{
    return countriesCodeUE(QDate::currentDate().year());
}
//----------------------------------------------------------
const QStringList *CountryManager::countriesCodeUE(int year)
{
    static QHash<int, QSharedPointer<QStringList>> countries
            = []() -> QHash<int, QSharedPointer<QStringList>> {
        QHash<int, QSharedPointer<QStringList>> countries;
        //QStringList countriesAfterBrexit = {"FR","MC","DE","IT","ES","PL","CZ","NL","SE","BE","BG","DK","EE","IE","GR","EL","HR","CY","LV","LT","LU","HU","MT","AT","PT","RO","SI","SK","FI"};
        QStringList countriesAfterBrexit = {"FR","MC","DE","IT","ES","PL","CZ","NL","SE","BE","BG","DK","EE","IE","EL","HR","CY","LV","LT","LU","HU","MT","AT","PT","RO","SI","SK","FI"};
        QStringList countriesBeforeBrexit = countriesAfterBrexit;
        countriesBeforeBrexit.append("GB");
        countriesAfterBrexit.append("GB-NIR");
        QSharedPointer<QStringList> pCountriesBeforeBrexit(new QStringList(countriesBeforeBrexit));
        for (int year = 2000; year <= 2020; ++year) {
            countries[year] = pCountriesBeforeBrexit;
        }
        QSharedPointer<QStringList> pCountriesAfterBrexit(new QStringList(countriesAfterBrexit));
        for (int year = 2021; year <= 2050; ++year) {
            countries[year] = pCountriesAfterBrexit;
        }
        return countries;
    }();
    return countries[year].data();
}
//----------------------------------------------------------
const QStringList *CountryManager::countriesNamesUE(int year)
{
    static QStringList countries
            = [year]() -> QStringList {
            QStringList countryNames;
            for (auto code : *countriesCodeUE(year)) {
                countryNames << CountryManager::instance()->countryName(code);
            }
            std::sort(countryNames.begin(), countryNames.end());
            return countryNames;
    }();
    return &countries;
}
//----------------------------------------------------------
const QStringList *CountryManager::countriesCodeUEfrom2020()
{
    static QStringList countries
            = []() -> QStringList {
        QSet<QString> countriesSet;
        for (int year = 2020; year <= QDate::currentDate().year(); ++year) {
            for (auto country : *countriesCodeUE(year)) {
                countriesSet << country;
            }
        }
        #if QT_VERSION <= 0x050000
        auto countries = countriesSet.toList();
        #else
        QStringList countries(countriesSet.begin(), countriesSet.end());
        #endif
        std::sort(countries.begin(), countries.end());
        return QStringList(countries);
    }();
return &countries;
}
//----------------------------------------------------------
const QStringList *CountryManager::countriesNamesUEfrom2020()
{
    static QStringList countries
            = []() -> QStringList {
            QStringList countryNames;
            for (auto code : *countriesCodeUEfrom2020()) {
                countryNames << CountryManager::instance()->countryName(code);
            }
            std::sort(countryNames.begin(), countryNames.end());
            return countryNames;
    }();
    return &countries;
}
//----------------------------------------------------------
const QStringList *CountryManager::months()
{
    static QStringList months
            = {QObject::tr("Janvier"), QObject::tr("Févier"), QObject::tr("Mars"),
              QObject::tr("Avril"), QObject::tr("Mai"), QObject::tr("Juin"),
              QObject::tr("Juillet"), QObject::tr("Août"), QObject::tr("Septembre"),
              QObject::tr("Octobre"), QObject::tr("Novembre"), QObject::tr("Décembre")};
    return &months;
}
//----------------------------------------------------------
const QHash<QString, CountryManager::CountryInfo> *CountryManager::countryInfos()
{
    static QHash<QString, CountryManager::CountryInfo> infos
            = {
        {"US", {"USD", "$", false}}
        , {"UK", {"GBP", "£", false}}
        , {"GB", {"GBP", "£", false}}
        , {"CA", {"CAD", "$", false}}
        , {"AU", {"AUD", "$", false}}
        , {"FR", {"EUR", "€", true}}
        , {"DE", {"EUR", "€", true}}
        , {"IT", {"EUR", "€", true}}
        , {"ES", {"EUR", "€", true}}
        , {"PH", {"PHP", "₱", false}}
    };
    return &infos;
}
//----------------------------------------------------------
const QString &CountryManager::getCurrencySymbol(
        const QString &countryCode) const
{
    return (*countryInfos()).find(countryCode).value().currencySymbol;
}
//----------------------------------------------------------
const QString &CountryManager::getCurrency(
        const QString &countryCode) const
{
    return (*countryInfos()).find(countryCode).value().currency;
}
//----------------------------------------------------------
const bool &CountryManager::getCurrencyAtEnd(
        const QString &countryCode) const
{
    return (*countryInfos()).find(countryCode).value().symbolAtEnd;
}
//----------------------------------------------------------
QString CountryManager::countryCodeDomTom(
        const QString &countryCode, const QString &postalCode) const
{
    if (countryCode == "FR") {
        if (postalCode.startsWith("971")) {
            return "GP";
        } else if (postalCode.startsWith("972")) {
            return "MQ";
        } else if (postalCode.startsWith("973")) {
            return "GF";
        } else if (postalCode.startsWith("974")) {
            return "RE";
        } else if (postalCode.startsWith("976")) {
            return "YT";
        }
    }
    return countryCode;
}
//----------------------------------------------------------
QString CountryManager::countryName(const QString &countryCode) const
{
    static QHash<QString, QString> codeToName
            = {{"FR", FRANCE},
               {"GB", UK},
               {"UK", UK},
               {EU, EU},
               {"DE", GERMANY},
               {"IT", ITALY},
               {"ES", SPAIN},
               {"PL", POLAND},
               {"CZ", CZECH},
               {"NL", NETHERLAND},
               {"SE", SWEDEN},
               {"BE", BELGIUM},
               {"BG", BULGARIA},
               {"DK", DENMARK},
               {"EE", ESTONIA},
               {"IE", IRELAND},
               {"GB-NIR", IRELAND_NORTHEN},
               {"EL", GREECE},
               {"GR", GREECE},
               {"HR", CROATIA},
               {"CY", CHYPRE},
               {"LV", LATVIA},
               {"LT", LITHUANIA},
               {"LU", LUXEMBOURG},
               {"MT", MALTA},
               {"PT", PORTUGAL},
               {"RO", ROMANIA},
               {"SI", SLOVENIA},
               {"SK", SLOVAKIA},
               {"FI", FINLAND},
               {"HU", QObject::tr("Hongrie")},
               {"AT", AUSTRIA},
               {"GP", QObject::tr("Guadeloupe")},
               {"MC", QObject::tr("Monaco")},
               {"IS", QObject::tr("Icelande")},
               {"CA", QObject::tr("Canada")},
               {"AE", QObject::tr("Émirats arabes unis")},
               {"US", QObject::tr("États-Unis")},
               {"RE", QObject::tr("Réunion")},
               {"MQ", QObject::tr("Martinique")},
               {"GF", QObject::tr("Guyane")},
               {"GP", QObject::tr("Guadeloupe")},
               {"YT", QObject::tr("Mayotte")},
               {"CH", QObject::tr("Suisse")},
               {"CN", CHINA},
               {"AF", QObject::tr("Afghanistan")},
               {"AL", QObject::tr("Albanie")},
               {"DZ", QObject::tr("Algérie")},
               {"AS", QObject::tr("Samoa américaines")},
               {"AD", QObject::tr("Andorre")},
               {"AO", QObject::tr("Angola")},
               {"AI", QObject::tr("Anguilla")},
               {"AG", QObject::tr("Antigua-et-Barbuda")},
               {"AR", QObject::tr("Argentine")},
               {"AM", QObject::tr("Arménie")},
               {"AW", QObject::tr("Aruba")},
               {"AU", QObject::tr("Australie")},
               {"AT", QObject::tr("Autriche")},
               {"AZ", QObject::tr("Azerbaïdjan")},
               {"BS", QObject::tr("Bahamas (les)")},
               {"BH", QObject::tr("Bahreïn")},
               {"BD", QObject::tr("Bengladesh")},
               {"BB", QObject::tr("Barbade")},
               {"BY", QObject::tr("Biélorussie")},
               {"BE", QObject::tr("Belgique")},
               {"BZ", QObject::tr("Bélize")},
               {"BJ", QObject::tr("Bénin")},
               {"BM", QObject::tr("Bermudes")},
               {"BT", QObject::tr("Bhoutan")},
               {"BO", QObject::tr("Bolivie")},
               {"BQ", QObject::tr("Bonaire, Saint Eustache et Saba")},
               {"BA", QObject::tr("Bosnie Herzégovine")},
               {"BW", QObject::tr("Bostwana")},
               {"BV", QObject::tr("Île Bouvet")},
               {"BR", QObject::tr("Brésil")},
               {"IO", QObject::tr("Territoire britannique de l'océan Indien")},
               {"BN", QObject::tr("Brunei Darussalam")},
               {"BG", QObject::tr("Bulgarie")},
               {"BF", QObject::tr("Burkina Faso")},
               {"BI", QObject::tr("Burundi")},
               {"CV", QObject::tr("Cap-Vert")},
               {"KH", QObject::tr("Cambodge")},
               {"CM", QObject::tr("Cameroun")},
               {"CA", QObject::tr("Canada")},
               {"KY", QObject::tr("Îles Caïmans")},
               {"CF", QObject::tr("République centrafricaine")},
               {"TD", QObject::tr("Tchad")},
               {"CL", QObject::tr("Chili")},
               {"CN", CHINA},
               {"CX", QObject::tr("L'île de noël")},
               {"CC", QObject::tr("Îles Cocos")},
               {"CO", QObject::tr("Colombie")},
               {"KM", QObject::tr("Comores")},
               {"CD", QObject::tr("Congo (République démocratique)")},
               {"CG", QObject::tr("Congo")},
               {"CK", QObject::tr("Îles Cook (les)")},
               {"CR", QObject::tr("Costa Rica")},
               {"HR", QObject::tr("Croatie")},
               {"CU", QObject::tr("Cuba")},
               {"CW", QObject::tr("Curacao")},
               {"CY", QObject::tr("Chypre")},
               {"CI", QObject::tr("Côte d'Ivoire")},
               {"DK", QObject::tr("Danemark")},
               {"DJ", QObject::tr("Djibouti")},
               {"DM", QObject::tr("Dominique")},
               {"DO", QObject::tr("République dominicaine")},
               {"EC", QObject::tr("Equateur")},
               {"EG", QObject::tr("Egypte")},
               {"SV", QObject::tr("Le Salvador")},
               {"GQ", QObject::tr("Guinée Équatoriale")},
               {"ER", QObject::tr("Érythrée")},
               {"EE", QObject::tr("Estonie")},
               {"SZ", QObject::tr("Eswatini")},
               {"ET", QObject::tr("Ethiopie")},
               {"FK", QObject::tr("Îles Falkland")},
               {"FO", QObject::tr("Îles Féroé")},
               {"FJ", QObject::tr("Fidji")},
               {"FI", QObject::tr("Finlande")},
               {"FR", QObject::tr("France")},
               {"GF", QObject::tr("Guyane Française")},
               {"PF", QObject::tr("Polynésie française")},
               {"TF", QObject::tr("Terres australes françaises")},
               {"GA", QObject::tr("Gabon")},
               {"GM", QObject::tr("Gambie (la)")},
               {"GE", QObject::tr("Géorgie")},
               {"DE", QObject::tr("Allemagne")},
               {"GH", QObject::tr("Ghana")},
               {"GI", QObject::tr("Gibraltar")},
               {"GR", QObject::tr("Grèce")},
               {"GD", QObject::tr("Grenade")},
               {"GP", QObject::tr("Guadeloupe")},
               {"GU", QObject::tr("Guam")},
               {"GT", QObject::tr("Guatemala")},
               {"GG", QObject::tr("Guernesey")},
               {"GN", QObject::tr("Guinée")},
               {"GW", QObject::tr("Guinée-Bissau")},
               {"GY", QObject::tr("Guyane")},
               {"HT", QObject::tr("Haïti")},
               {"HM", QObject::tr("Île Heard et îles McDonald")},
               {"VA", QObject::tr("Saint-Siège")},
               {"HN", QObject::tr("Honduras")},
               {"HK", QObject::tr("Hong Kong")},
               {"HU", QObject::tr("Hongrie")},
               {"IS", QObject::tr("Islande")},
               {"IN", QObject::tr("Inde")},
               {"ID", QObject::tr("Indonésie")},
               {"IR", QObject::tr("Iran")},
               {"IQ", QObject::tr("Irak")},
               {"IM", QObject::tr("île de Man")},
               {"IL", QObject::tr("Israël")},
               {"IT", QObject::tr("Italie")},
               {"JM", QObject::tr("Jamaïque")},
               {"JP", QObject::tr("Japon")},
               {"JE", QObject::tr("Jersey")},
               {"JO", QObject::tr("Jordan")},
               {"KZ", QObject::tr("Kazakhstan")},
               {"KE", QObject::tr("Kenya")},
               {"KI", QObject::tr("Kiribati")},
               {"KP", QObject::tr("Corée du nord")},
               {"KR", QObject::tr("Corée du sud")},
               {"KW", QObject::tr("Koweit")},
               {"KG", QObject::tr("Kirghizistan")},
               {"LA", QObject::tr("Laos")},
               {"LV", QObject::tr("Lettonie")},
               {"LB", QObject::tr("Liban")},
               {"LS", QObject::tr("Lesotho")},
               {"LR", QObject::tr("Libéria")},
               {"LY", QObject::tr("Libye")},
               {"LI", QObject::tr("Liechtenstein")},
               {"LT", QObject::tr("Lituanie")},
               {"LU", QObject::tr("Luxembourg")},
               {"MO", QObject::tr("Macao")},
               {"MG", QObject::tr("Madagascar")},
               {"MW", QObject::tr("Malawi")},
               {"MY", QObject::tr("Malaisie")},
               {"MV", QObject::tr("Maldives")},
               {"ML", QObject::tr("Mali")},
               {"MT", QObject::tr("Malte")},
               {"MH", QObject::tr("Îles Marshall (les)")},
               {"MQ", QObject::tr("Martinique")},
               {"MR", QObject::tr("Mauritanie")},
               {"MU", QObject::tr("Maurice")},
               {"YT", QObject::tr("Mayotte")},
               {"MX", QObject::tr("Mexique")},
               {"FM", QObject::tr("Micronésie")},
               {"MD", QObject::tr("Moldavie")},
               {"MC", QObject::tr("Monaco")},
               {"MN", QObject::tr("Mongolie")},
               {"ME", QObject::tr("Monténégro")},
               {"MS", QObject::tr("Montserrat")},
               {"MA", QObject::tr("Maroc")},
               {"MZ", QObject::tr("Mozambique")},
               {"MM", QObject::tr("Birmanie")},
               {"NA", QObject::tr("Namibie")},
               {"NR", QObject::tr("Nauru")},
               {"NP", QObject::tr("Népal")},
               {"NC", QObject::tr("Nouvelle Calédonie")},
               {"NZ", QObject::tr("Nouvelle-Zélande")},
               {"NI", QObject::tr("Nicaragua")},
               {"NE", QObject::tr("Niger (le)")},
               {"NG", QObject::tr("Nigeria")},
               {"NU", QObject::tr("Niué")},
               {"NF", QObject::tr("l'ile de Norfolk")},
               {"MP", QObject::tr("Îles Mariannes du Nord")},
               {"NO", QObject::tr("Norvège")},
               {"OM", QObject::tr("Oman")},
               {"PK", QObject::tr("Pakistan")},
               {"PW", QObject::tr("Palaos")},
               {"PS", QObject::tr("Palestine, État de")},
               {"PA", QObject::tr("Panama")},
               {"PG", QObject::tr("Papouasie Nouvelle Guinée")},
               {"PY", QObject::tr("Paraguay")},
               {"PE", QObject::tr("Pérou")},
               {"PH", QObject::tr("Philippines (les)")},
               {"PN", QObject::tr("Pitcairn")},
               {"PL", QObject::tr("Pologne")},
               {"PR", QObject::tr("Porto Rico")},
               {"QA", QObject::tr("Qatar")},
               {"MK", QObject::tr("République de Macédoine du Nord")},
               {"RO", QObject::tr("Roumanie")},
               {"RU", QObject::tr("Fédération de Russie (la)")},
               {"RW", QObject::tr("Rwanda")},
               {"RE", QObject::tr("Réunion")},
               {"BL", QObject::tr("Saint Barthélemy")},
               {"SH", QObject::tr("Sainte-Hélène, Ascension et Tristan da Cunha")},
               {"KN", QObject::tr("Saint-Christophe-et-Niévès")},
               {"LC", QObject::tr("Sainte-Lucie")},
               {"MF", QObject::tr("Saint Martin (partie française)")},
               {"PM", QObject::tr("Saint-Pierre-et-Miquelon")},
               {"VC", QObject::tr("Saint-Vincent-et-les-Grenadines")},
               {"WS", QObject::tr("Samoa")},
               {"SM", QObject::tr("Saint Marin")},
               {"ST", QObject::tr("Sao Tomé et Principe")},
               {"SA", QObject::tr("Arabie Saoudite")},
               {"SN", QObject::tr("Sénégal")},
               {"RS", QObject::tr("Serbie")},
               {"SC", QObject::tr("les Seychelles")},
               {"SL", QObject::tr("Sierra Leone")},
               {"SG", QObject::tr("Singapour")},
               {"SX", QObject::tr("Sint Maarten")},
               {"SK", QObject::tr("Slovaquie")},
               {"SI", QObject::tr("Slovénie")},
               {"SB", QObject::tr("îles Salomon")},
               {"SO", QObject::tr("Somalie")},
               {"ZA", QObject::tr("Afrique du Sud")},
               {"SS", QObject::tr("Soudan du sud")},
               {"ES", QObject::tr("Espagne")},
               {"LK", QObject::tr("Sri Lanka")},
               {"SD", QObject::tr("Soudan")},
               {"SR", QObject::tr("Suriname")},
               {"SJ", QObject::tr("Svalbard et Jan Mayen")},
               {"SE", QObject::tr("Suède")},
               {"CH", QObject::tr("Suisse")},
               {"SY", QObject::tr("République arabe syrienne")},
               {"TW", QObject::tr("Taiwan")},
               {"TJ", QObject::tr("Tadjikistan")},
               {"TZ", QObject::tr("Tanzanie, République-Unie de")},
               {"TH", QObject::tr("Thaïlande")},
               {"TL", QObject::tr("Timor oriental")},
               {"TG", QObject::tr("Aller")},
               {"TK", QObject::tr("Tokélaou")},
               {"TO", QObject::tr("Tonga")},
               {"TT", QObject::tr("Trinité-et-Tobago")},
               {"TN", QObject::tr("Tunisie")},
               {"TR", QObject::tr("Turquie")},
               {"TM", QObject::tr("Turkménistan")},
               {"TC", QObject::tr("Îles Turques et Caïques")},
               {"TV", QObject::tr("Tuvalu")},
               {"UG", QObject::tr("Ouganda")},
               {"UA", QObject::tr("Ukraine")},
               {"AE", QObject::tr("Emirats Arabes Unis")},
               {"GB", UK}, //QObject::tr("Royaume-Uni de Grande-Bretagne et d'Irlande du Nord")},
               {"UM", QObject::tr("Îles mineures éloignées des États-Unis")},
               {"US", QObject::tr("États-Unis")},
               {"UY", QObject::tr("Uruguay")},
               {"UZ", QObject::tr("Ouzbékistan")},
               {"VU", QObject::tr("Vanuatu")},
               {"VE", QObject::tr("Venezuela")},
               {"VN", QObject::tr("Vietnam")},
               {"VG", QObject::tr("Îles Vierges britanniques")},
               {"VI", QObject::tr("Îles Vierges des États-Unis")},
               {"WF", QObject::tr("Wallis et Futuna")},
               {"EH", QObject::tr("Sahara occidental")},
               {"YE", QObject::tr("Yémen")},
               {"ZM", QObject::tr("Zambie")},
               {"ZW", QObject::tr("Zimbabwe")},
               {"AX", QObject::tr("Iles Aland")},
               {"AF", QObject::tr("Afghanistan")},
               {"AL", QObject::tr("Albanie")},
               {"DZ", QObject::tr("Algérie")},
               {"AS", QObject::tr("Samoa américaines")},
               {"AD", QObject::tr("Andorre")},
               {"AO", QObject::tr("Angola")},
               {"AI", QObject::tr("Anguilla")},
               {"AG", QObject::tr("Antigua-et-Barbuda")},
               {"AR", QObject::tr("Argentine")},
               {"AM", QObject::tr("Arménie")},
               {"AW", QObject::tr("Aruba")},
               {"AU", QObject::tr("Australie")},
               {"AZ", QObject::tr("Azerbaïdjan")},
               {"BS", QObject::tr("Bahamas (les)")},
               {"BH", QObject::tr("Bahreïn")},
               {"BD", QObject::tr("Bengladesh")},
               {"BB", QObject::tr("Barbade")},
               {"BY", QObject::tr("Biélorussie")},
               {"BZ", QObject::tr("Bélize")},
               {"BJ", QObject::tr("Bénin")},
               {"BM", QObject::tr("Bermudes")},
               {"BT", QObject::tr("Bhoutan")},
               {"BO", QObject::tr("Bolivie")},
               {"BQ", QObject::tr("Bonaire, Saint Eustache et Saba")},
               {"BA", QObject::tr("Bosnie Herzégovine")},
               {"BW", QObject::tr("Bostwana")},
               {"BV", QObject::tr("Île Bouvet")},
               {"BR", QObject::tr("Brésil")},
               {"IO", QObject::tr("Territoire britannique de l'océan Indien")},
               {"BN", QObject::tr("Brunei Darussalam")},
               {"BF", QObject::tr("Burkina Faso")},
               {"BI", QObject::tr("Burundi")},
               {"CV", QObject::tr("Cap-Vert")},
               {"KH", QObject::tr("Cambodge")},
               {"CM", QObject::tr("Cameroun")},
               {"KY", QObject::tr("Îles Caïmans")},
               {"CF", QObject::tr("République centrafricaine")},
               {"TD", QObject::tr("Tchad")},
               {"CL", QObject::tr("Chili")},
               {"CX", QObject::tr("L'île de noël")},
               {"CC", QObject::tr("Îles Cocos")},
               {"CO", QObject::tr("Colombie")},
               {"KM", QObject::tr("Comores")},
               {"CD", QObject::tr("Congo (République démocratique)")},
               {"CG", QObject::tr("Congo")},
               {"CK", QObject::tr("Îles Cook (les)")},
               {"CR", QObject::tr("Costa Rica")},
               {"CU", QObject::tr("Cuba")},
               {"CW", QObject::tr("Curacao")},
               {"CI", QObject::tr("Côte d'Ivoire")},
               {"DJ", QObject::tr("Djibouti")},
               {"DM", QObject::tr("Dominique")},
               {"DO", QObject::tr("République dominicaine")},
               {"EC", QObject::tr("Equateur")},
               {"EG", QObject::tr("Egypte")},
               {"SV", QObject::tr("Le Salvador")},
               {"GQ", QObject::tr("Guinée Équatoriale")},
               {"ER", QObject::tr("Érythrée")},
               {"SZ", QObject::tr("Eswatini")},
               {"ET", QObject::tr("Ethiopie")},
               {"FK", QObject::tr("Îles Falkland")},
               {"FO", QObject::tr("Îles Féroé")},
               {"FJ", QObject::tr("Fidji")},
               {"PF", QObject::tr("Polynésie française")},
               {"TF", QObject::tr("Terres australes françaises")},
               {"GA", QObject::tr("Gabon")},
               {"GM", QObject::tr("Gambie (la)")},
               {"GE", QObject::tr("Géorgie")},
               {"GH", QObject::tr("Ghana")},
               {"GI", QObject::tr("Gibraltar")},
               {"GD", QObject::tr("Grenade")},
               {"GU", QObject::tr("Guam")},
               {"GT", QObject::tr("Guatemala")},
               {"GG", QObject::tr("Guernesey")},
               {"GN", QObject::tr("Guinée")},
               {"GW", QObject::tr("Guinée-Bissau")},
               {"GY", QObject::tr("Guyane")},
               {"HT", QObject::tr("Haïti")},
               {"HM", QObject::tr("Île Heard et îles McDonald")},
               {"VA", QObject::tr("Saint-Siège")},
               {"HN", QObject::tr("Honduras")},
               {"HK", QObject::tr("Hong Kong")},
               {"IN", QObject::tr("Inde")},
               {"ID", QObject::tr("Indonésie")},
               {"IR", QObject::tr("Iran")},
               {"IQ", QObject::tr("Irak")},
               {"IM", QObject::tr("île de Man")},
               {"IL", QObject::tr("Israël")},
               {"JM", QObject::tr("Jamaïque")},
               {"JP", QObject::tr("Japon")},
               {"JE", QObject::tr("Jersey")},
               {"JO", QObject::tr("Jordan")},
               {"KZ", QObject::tr("Kazakhstan")},
               {"KE", QObject::tr("Kenya")},
               {"KI", QObject::tr("Kiribati")},
               {"KP", QObject::tr("Corée du nord")},
               {"KR", QObject::tr("Corée du sud")},
               {"KW", QObject::tr("Koweit")},
               {"KG", QObject::tr("Kirghizistan")},
               {"LA", QObject::tr("Laos")},
               {"LB", QObject::tr("Liban")},
               {"LS", QObject::tr("Lesotho")},
               {"LR", QObject::tr("Libéria")},
               {"LY", QObject::tr("Libye")},
               {"LI", QObject::tr("Liechtenstein")},
               {"MO", QObject::tr("Macao")},
               {"MG", QObject::tr("Madagascar")},
               {"MW", QObject::tr("Malawi")},
               {"MY", QObject::tr("Malaisie")},
               {"MV", QObject::tr("Maldives")},
               {"ML", QObject::tr("Mali")},
               {"MH", QObject::tr("Îles Marshall (les)")},
               {"MR", QObject::tr("Mauritanie")},
               {"MU", QObject::tr("Maurice")},
               {"MX", QObject::tr("Mexique")},
               {"FM", QObject::tr("Micronésie")},
               {"MD", QObject::tr("Moldavie")},
               {"MN", QObject::tr("Mongolie")},
               {"ME", QObject::tr("Monténégro")},
               {"MS", QObject::tr("Montserrat")},
               {"MA", QObject::tr("Maroc")},
               {"MZ", QObject::tr("Mozambique")},
               {"MM", QObject::tr("Birmanie")},
               {"NA", QObject::tr("Namibie")},
               {"NR", QObject::tr("Nauru")},
               {"NP", QObject::tr("Népal")},
               {"NC", QObject::tr("Nouvelle Calédonie")},
               {"NZ", QObject::tr("Nouvelle-Zélande")},
               {"NI", QObject::tr("Nicaragua")},
               {"NE", QObject::tr("Niger (le)")},
               {"NG", QObject::tr("Nigeria")},
               {"NU", QObject::tr("Niué")},
               {"NF", QObject::tr("l'ile de Norfolk")},
               {"MP", QObject::tr("Îles Mariannes du Nord")},
               {"NO", QObject::tr("Norvège")},
               {"OM", QObject::tr("Oman")},
               {"PK", QObject::tr("Pakistan")},
               {"PW", QObject::tr("Palaos")},
               {"PS", QObject::tr("Palestine, État de")},
               {"PA", QObject::tr("Panama")},
               {"PG", QObject::tr("Papouasie Nouvelle Guinée")},
               {"PY", QObject::tr("Paraguay")},
               {"PE", QObject::tr("Pérou")},
               {"PH", QObject::tr("Philippines (les)")},
               {"PN", QObject::tr("Pitcairn")},
               {"PR", QObject::tr("Porto Rico")},
               {"QA", QObject::tr("Qatar")},
               {"MK", QObject::tr("République de Macédoine du Nord")},
               {"RU", QObject::tr("Fédération de Russie (la)")},
               {"RW", QObject::tr("Rwanda")},
               {"BL", QObject::tr("Saint Barthélemy")},
               {"SH", QObject::tr("Sainte-Hélène, Ascension et Tristan da Cunha")},
               {"KN", QObject::tr("Saint-Christophe-et-Niévès")},
               {"LC", QObject::tr("Sainte-Lucie")},
               {"MF", QObject::tr("Saint Martin (partie française)")},
               {"PM", QObject::tr("Saint-Pierre-et-Miquelon")},
               {"VC", QObject::tr("Saint-Vincent-et-les-Grenadines")},
               {"WS", QObject::tr("Samoa")},
               {"SM", QObject::tr("Saint Marin")},
               {"ST", QObject::tr("Sao Tomé et Principe")},
               {"SA", QObject::tr("Arabie Saoudite")},
               {"SN", QObject::tr("Sénégal")},
               {"RS", QObject::tr("Serbie")},
               {"SC", QObject::tr("les Seychelles")},
               {"SL", QObject::tr("Sierra Leone")},
               {"SG", QObject::tr("Singapour")},
               {"SX", QObject::tr("Sint Maarten")},
               {"SB", QObject::tr("îles Salomon")},
               {"SO", QObject::tr("Somalie")},
               {"ZA", QObject::tr("Afrique du Sud")},
               {"SS", QObject::tr("Soudan du sud")},
               {"LK", QObject::tr("Sri Lanka")},
               {"SD", QObject::tr("Soudan")},
               {"SR", QObject::tr("Suriname")},
               {"SJ", QObject::tr("Svalbard et Jan Mayen")},
               {"SY", QObject::tr("République arabe syrienne")},
               {"TW", QObject::tr("Taiwan")},
               {"TJ", QObject::tr("Tadjikistan")},
               {"TZ", QObject::tr("Tanzanie, République-Unie de")},
               {"TH", QObject::tr("Thaïlande")},
               {"TL", QObject::tr("Timor oriental")},
               {"TG", QObject::tr("Aller")},
               {"TK", QObject::tr("Tokélaou")},
               {"TO", QObject::tr("Tonga")},
               {"TT", QObject::tr("Trinité-et-Tobago")},
               {"TN", QObject::tr("Tunisie")},
               {"TR", QObject::tr("Turquie")},
               {"TM", QObject::tr("Turkménistan")},
               {"TC", QObject::tr("Îles Turques et Caïques")},
               {"TV", QObject::tr("Tuvalu")},
               {"UG", QObject::tr("Ouganda")},
               {"UA", QObject::tr("Ukraine")},
               {"UM", QObject::tr("Îles mineures éloignées des États-Unis")},
               {"UY", QObject::tr("Uruguay")},
               {"UZ", QObject::tr("Ouzbékistan")},
               {"VU", QObject::tr("Vanuatu")},
               {"VE", QObject::tr("Venezuela")},
               {"VN", QObject::tr("Vietnam")},
               {"VG", QObject::tr("Îles Vierges britanniques")},
               {"VI", QObject::tr("Îles Vierges des États-Unis")},
               {"WF", QObject::tr("Wallis et Futuna")},
               {"EH", QObject::tr("Sahara occidental")},
               {"YE", QObject::tr("Yémen")},
               {"ZM", QObject::tr("Zambie")},
               {"ZW", QObject::tr("Zimbabwe")},
               {"AX", QObject::tr("Iles Aland")},
               {"", ""}
              };
    //Q_ASSERT(!countryCode.isEmpty());
    //Q_ASSERT(codeToName.contains(countryCode.toUpper()));
    return codeToName[countryCode.toUpper()];
}
//----------------------------------------------------------
QString CountryManager::countryCode(const QString &name) const
{
    static QHash<QString, QString> codeToName = [this]() -> QHash<QString, QString>{
        QHash<QString, QString> nameToCode;
        for (auto code : *countryCodes()) {
            //nameToCode[code] = code;
            nameToCode[countryName(code)] = code;
        }
        nameToCode[EU] = EU;
        return nameToCode;
    }();
    return codeToName[name];
}
//----------------------------------------------------------
