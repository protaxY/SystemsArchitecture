--[[
0.2 - /user
    0.25 - get all
    0.75 - get by id
0.4 - /message
    0.25 - get all
    0.75 - get by id
0.4 - /post
    0.25 - get all
    0.75 - get by id
]]

math.randomseed(0)

users_count = 100
users = {
    rachelblankenship,
    sanchezrobert,
    osmith,
    zmartin,
    garciakenneth,
    roger00,
    sarahwagner,
    marystewart,
    kanedeanna,
    ygrimes,
    melissaford,
    welcherica,
    wintersamanda,
    williamwelch,
    xthomas,
    joy60,
    teresaevans,
    julieburke,
    clarkegregory,
    ramoselizabeth,
    martinezmichael,
    natasha81,
    wardmichael,
    mmiller,
    mhunt,
    kelseyhicks,
    hallkimberly,
    gmiller,
    rlindsey,
    ibennett,
    smoyer,
    clopez,
    award,
    torreskyle,
    christophergibson,
    larsonbrandi,
    lopezsean,
    tbryant,
    megan47,
    mfitzgerald,
    sarabailey,
    karen41,
    matthewkelly,
    riggsjessica,
    tammy97,
    tbarnes,
    jasonstone,
    mccallbenjamin,
    sawyerjessica,
    troy84,
    sbaker,
    vclayton,
    katherineaguilar,
    wkeller,
    richard11,
    gregory73,
    dwilliams,
    garylane,
    brittneyhenderson,
    daviscorey,
    bartonnicole,
    robertrandall,
    elizabethstokes,
    lopezamy,
    stephanie70,
    angelnunez,
    crystal62,
    hesterjennifer,
    stephanie01,
    kimberly61,
    mcintyrejohn,
    nbowman,
    heather06,
    oestrada,
    melissapaul,
    courtneypeters,
    mezapatricia,
    martinezmariah,
    william13,
    john51,
    rogerjohnson,
    meadowsdaniel,
    olsonsean,
    xrobinson,
    jill65,
    jose21,
    ispencer,
    pbarnes,
    benjaminjames,
    keith51,
    todd34,
    mary90,
    eray,
    zhernandez,
    twilson,
    dhunt,
    daniellejohnson,
    ptravis,
    joneskimberly,
    jim30
}
messages_count = 1000
posts_count = 1000

headers = {["Authorization"] = "Basic amltMzA6cGFzc3dvcmQ="} -- login:password
body = ""

request = function()
    path = math.random()
    req = math.random()

    if path < 0.2 then -- /user
        if req < 0.25 then -- get all
            return wrk.format("GET", "/user", headers, body)
        else -- get by id
            id = math.random(users_count) - 1
            return wrk.format("GET", string.format("/user?login=%s", users[id]), headers, body)
        end
    elseif path < 0.6 then -- /message
        if req < 0.25 then -- get all
            id = math.random(messages_count) - 1
            return wrk.format("GET", "/message", headers, body)
        else -- get chat messages
            id = math.random(messages_count)
            return wrk.format("GET", string.format("/message?id=%d", id), headers, body)
        end
    else -- /post
        if req < 0.25 then -- get all
            id = math.random(posts_count) - 1
            return wrk.format("GET", "/post", headers, body)
        else -- get chat messages
            id = math.random(posts_count)
            return wrk.format("GET", string.format("/post?id=%d", id), headers, body)
        end
    end
end