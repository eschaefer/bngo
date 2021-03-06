[@bs.val] external unsafeJsonParse: string => 'a = "JSON.parse";

let localStorageNamespace = "tracks";

type favorite = {
  id: int,
  title: string,
  artwork_url: option(string),
  stream_url: option(string),
  streamable: bool,
  duration: int,
};

type user = {id: int};

type following = {
  id: int,
  username: string,
  permalink: string,
};

type followings = {collection: list(following)};

type ownedFavorite = {
  owner: following,
  favorite,
};

/* State declaration */
type state = {
  isPlayerPlaying: bool,
  isLoading: bool,
  isLoadingComplete: bool,
  username: string,
  userId: int,
  followings: list(following),
  favorites: list(ownedFavorite),
  currentTrack: Track.track,
};

/* Action declaration */
type action =
  | ToggleLoading(bool)
  | Play(Track.track)
  | PlayNext
  | TogglePlay
  | SetUsername(string)
  | SetUserId(int)
  | SetFollowings(list(following))
  | SetFavorites(list(favorite), following);

let initialFavorites =
  switch (Dom.Storage.(localStorage |> getItem(localStorageNamespace))) {
  | None => []
  | Some(favs) => unsafeJsonParse(favs)
  };

let initialCurrentTrack: Track.track = {
  id: 0,
  title: "",
  artwork_url: "",
  stream_url: "",
  bumper: "",
  bumperLink: "",
  duration: 0,
  duration_readable: "",
};

let initialState = {
  isPlayerPlaying: false,
  isLoading: false,
  isLoadingComplete: false,
  username: "",
  userId: 0,
  followings: [],
  favorites: initialFavorites,
  currentTrack: initialCurrentTrack,
};

let clientId = "b1cff2065031b0c616f44fc3f972fa0a";

let padStart = value =>
  if (value < 10) {
    "0" ++ string_of_int(value);
  } else {
    string_of_int(value);
  };

let durationConverter = secs => {
  let minutes = secs mod 3600 / 60 |> padStart;
  let seconds = secs mod 60 |> padStart;
  let hours = secs / 3600;

  if (hours < 1) {
    minutes ++ ":" ++ seconds;
  } else {
    string_of_int(hours) ++ ":" ++ minutes ++ ":" ++ seconds;
  };
};

module Decode = {
  let user = json => Json.Decode.{id: json |> field("id", int)};
  let following = json =>
    Json.Decode.{
      id: json |> field("id", int),
      username: json |> field("username", string),
      permalink: json |> field("permalink", string),
    };
  let followings = json =>
    Json.Decode.{collection: json |> field("collection", list(following))};
  let favorite = json =>
    Json.Decode.{
      id: json |> field("id", int),
      title: json |> field("title", string),
      artwork_url: json |> optional(field("artwork_url", string)),
      stream_url: json |> optional(field("stream_url", string)),
      streamable: json |> field("streamable", bool),
      duration: json |> field("duration", int),
    };

  let userFavorites = json =>
    Json.Decode.array(favorite, json) |> Array.to_list;
};

let trackFromFavorite = (favorite: ownedFavorite): Track.track => {
  title: favorite.favorite.title,
  id: favorite.favorite.id,
  bumper: favorite.owner.username,
  bumperLink: favorite.owner.permalink,
  duration: favorite.favorite.duration / 1000,
  duration_readable: durationConverter(favorite.favorite.duration / 1000),
  artwork_url: {
    // Wow
    let art =
      Js.Nullable.toOption(
        Js.Nullable.fromOption(favorite.favorite.artwork_url),
      );

    switch (art) {
    | Some(url) => url
    | None => ""
    };
  },

  stream_url:
    switch (favorite.favorite.stream_url) {
    | Some(url) => url
    | None => ""
    },
};

let getNextTrack = state => {
  let defaultFav: ownedFavorite = {
    owner: {
      id: 0,
      username: "",
      permalink: "",
    },
    favorite: {
      id: 0,
      title: "",
      artwork_url: None,
      stream_url: None,
      streamable: true,
      duration: 0,
    },
  };

  let nextTrack =
    Belt.List.reduceWithIndex(state.favorites, 0, (acc, x, i) =>
      if (state.currentTrack.id == x.favorite.id) {
        i + 1;
      } else {
        acc;
      }
    )
    |> Belt.List.get(state.favorites)
    |> (
      result =>
        switch (result) {
        | Some(res) => res
        | None => defaultFav
        }
    );

  trackFromFavorite(nextTrack);
};

[@react.component]
let make = () => {
  let (state, dispatch) =
    React.useReducer(
      (state, action) =>
        switch (action) {
        | ToggleLoading(value) => {
            ...state,
            isLoading: value,
            favorites: value === true ? [] : state.favorites,
          }
        | Play(track) => {
            ...state,
            isPlayerPlaying: true,
            currentTrack: track,
          }
        | PlayNext => {...state, currentTrack: getNextTrack(state)}
        | TogglePlay => {...state, isPlayerPlaying: !state.isPlayerPlaying}
        | SetUsername(username) => {...state, username}
        | SetUserId(userId) => {...state, userId}
        | SetFollowings(followings) => {...state, followings}
        | SetFavorites(favorites, following) => {
            ...state,
            isLoadingComplete: true,
            favorites:
              List.append(
                state.favorites,
                List.filter(
                  favorite =>
                    switch (favorite.stream_url, favorite.streamable) {
                    | (Some(url), true) => String.length(url) > 0
                    | (Some(_), false) => false
                    | (None, _) => false
                    },
                  favorites,
                )
                |> List.map(favorite => {owner: following, favorite}),
              ),
          }
        },
      initialState,
    );

  React.useEffect1(
    () => {
      // Get the user info when username is submitted.
      if (String.length(state.username) > 0) {
        dispatch(ToggleLoading(true));
        Js.Promise.(
          Fetch.fetch(
            "https://api.soundcloud.com/resolve?url=http://soundcloud.com/"
            ++ state.username
            ++ "&client_id="
            ++ clientId,
          )
          |> then_(Fetch.Response.json)
          |> then_(json => Decode.user(json) |> (user => user.id |> resolve))
          |> then_(user => dispatch(SetUserId(user)) |> resolve)
          |> ignore
        );
      };

      None;
    },
    [|state.username|],
  );

  React.useEffect1(
    () => {
      // Get the user's followings when the user id has been set
      if (state.userId > 0) {
        Js.Promise.(
          Fetch.fetch(
            "https://api.soundcloud.com/users/"
            ++ string_of_int(state.userId)
            ++ "/followings?client_id="
            ++ clientId,
          )
          |> then_(Fetch.Response.json)
          |> then_(json =>
               Decode.followings(json)
               |> (followings => followings.collection |> resolve)
             )
          |> then_(followings =>
               dispatch(SetFollowings(followings)) |> resolve
             )
          |> ignore
        );
      };

      None;
    },
    [|state.userId|],
  );

  React.useEffect1(
    () => {
      // Now get favorites from each following
      if (List.length(state.followings) > 0) {
        let _ =
          state.followings
          |> Array.of_list
          |> Array.map((follower: following) =>
               Js.Promise.(
                 Fetch.fetch(
                   "https://api.soundcloud.com/users/"
                   ++ string_of_int(follower.id)
                   ++ "/favorites?client_id="
                   ++ clientId,
                 )
                 |> then_(Fetch.Response.json)
                 |> then_(json => Decode.userFavorites(json) |> resolve)
                 |> then_(favs =>
                      dispatch(SetFavorites(favs, follower)) |> resolve
                    )
               )
             )
          |> Js.Promise.all
          |> Js.Promise.then_(_ =>
               dispatch(ToggleLoading(false)) |> Js.Promise.resolve
             )
          |> ignore;
        ();
      };

      None;
    },
    [|state.followings|],
  );

  React.useEffect2(
    () => {
      if (state.isLoadingComplete) {
        let favs =
          switch (Js.Json.stringifyAny(state.favorites)) {
          | Some(favs) => favs
          | None => ""
          };

        Dom.Storage.(localStorage |> setItem(localStorageNamespace, favs));
      };

      None;
    },
    (state.isLoadingComplete, state.favorites),
  );

  let trackList =
    React.array(
      Belt.List.mapWithIndex(
        state.favorites,
        (index, fav) => {
          // Define this since typings will be wrong if favorite is passed directly to the <Track>
          let track: Track.track = trackFromFavorite(fav);

          <Track
            key={string_of_int(index) ++ string_of_int(fav.favorite.id)}
            track
            setUsername={username => dispatch(SetUsername(username))}
            currentTrack={state.currentTrack}
            isPlayerPlaying={state.isPlayerPlaying}
            onTogglePlay={_ => dispatch(TogglePlay)}
            onPlay={track => dispatch(Play(track))}
          />;
        },
      )
      |> Array.of_list
      |> Js.Array.slice(~start=0, ~end_=300) // Just cap the list at this number for now
    );

  <>
    <Player
      clientId
      currentTrack={state.currentTrack}
      isPlayerPlaying={state.isPlayerPlaying}
      onTogglePlay={_ => dispatch(TogglePlay)}
      onPlayNext={_ => dispatch(PlayNext)}
    />
    <Form
      isLoading={state.isLoading}
      handleSubmit={username => dispatch(SetUsername(username))}
    />
    <div>
      {if (List.length(state.favorites) === 0 && !state.isLoading) {
         <Defaults
           setUsername={username => dispatch(SetUsername(username))}
         />;
       } else {
         trackList;
       }}
    </div>
  </>;
};