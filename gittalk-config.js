/*gittalk id must be less than 50*/
;(function($) {
    "use strict";

    var pathname = window.location.pathname;
    pathname = pathname.split('/');
    var id = pathname[pathname.length - 1];

    if (id.length == 0) {
        id = pathname[pathname.length - 2];
    }

    // handler 1: delete the _ character.
    if (id.length > 50) {
        id = id.split('_').join('')
    } 

    // handler 2: use md5.
    if (id.length > 50) {
        id = md5(id);
    }

    var gitalk = new Gitalk({
        "clientID": "7bf1e91eaff35fd004dc",
        "clientSecret": "d151c7bc85d544dafe2c0de75e3a174623229a3a",
        "repo": "wangwei1237.github.io",
        "owner": "wangwei1237",
        "admin": ["wangwei1237"],
        "id": id, 
        "distractionFreeMode": false
    });

    gitalk.render("gitalk-container");
}) (this)

