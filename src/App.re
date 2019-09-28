[@bs.val] external unsafeJsonParse: string => 'a = "JSON.parse";

let localStorageNamespace = "tracks";

type favorite = {
  id: int,
  title: string,
  artwork_url: option(string),
  stream_url: option(string),
};

type user = {id: int};

type following = {
  id: int,
  username: string,
};

type followings = {collection: list(following)};

/* State declaration */
type state = {
  isPlayerPlaying: bool,
  isLoading: bool,
  isLoadingComplete: bool,
  username: string,
  userId: int,
  followings: list(following),
  favorites: list(favorite),
  currentTrack: Track.track,
};

/* Action declaration */
type action =
  | ToggleLoading(bool)
  | Play(Track.track)
  | TogglePlay
  | SetUsername(string)
  | SetUserId(int)
  | SetFollowings(list(following))
  | SetFavorites(list(favorite));

let initialFavorites =
  switch (Dom.Storage.(localStorage |> getItem(localStorageNamespace))) {
  | None => []
  | Some(favs) => unsafeJsonParse(favs)
  };

let initialState = {
  isPlayerPlaying: false,
  isLoading: false,
  isLoadingComplete: false,
  username: "",
  userId: 0,
  followings: [],
  favorites: initialFavorites,
  currentTrack: {
    id: 0,
    title: "",
    artwork_url: "",
    stream_url: "",
  },
};

let clientId = "b1cff2065031b0c616f44fc3f972fa0a";

module Decode = {
  let crazy_artwork = json =>
    switch (Js.Json.stringifyAny(json)) {
    | Some(url) => String.length(url) > 5 ? url : ""
    | None => ""
    };

  let user = json => Json.Decode.{id: json |> field("id", int)};
  let following = json =>
    Json.Decode.{
      id: json |> field("id", int),
      username: json |> field("username", string),
    };
  let followings = json =>
    Json.Decode.{collection: json |> field("collection", list(following))};
  let favorite = json =>
    Json.Decode.{
      id: json |> field("id", int),
      title: json |> field("title", string),
      artwork_url: json |> optional(field("artwork_url", crazy_artwork)),
      stream_url: json |> optional(field("stream_url", string)),
    };
  let userFavorites = json =>
    Json.Decode.array(favorite, json) |> Array.to_list;
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
        | TogglePlay => {...state, isPlayerPlaying: !state.isPlayerPlaying}
        | SetUsername(username) => {...state, username}
        | SetUserId(userId) => {...state, userId}
        | SetFollowings(followings) => {...state, followings}
        | SetFavorites(favorites) => {
            ...state,
            isLoadingComplete: true,
            favorites:
              List.append(
                state.favorites,
                List.filter(
                  favorite =>
                    switch (favorite.stream_url) {
                    | Some(url) => String.length(url) > 0
                    | None => false
                    },
                  favorites,
                ),
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
                 |> then_(favs => dispatch(SetFavorites(favs)) |> resolve)
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

  <>
    <Player
      clientId
      currentTrack={state.currentTrack}
      isPlayerPlaying={state.isPlayerPlaying}
      onTogglePlay={_ => dispatch(TogglePlay)}
    />
    <Form
      isLoading={state.isLoading}
      handleSubmit={username => dispatch(SetUsername(username))}
    />
    <div>
      {React.array(
         Belt.List.mapWithIndex(
           state.favorites,
           (index, fav) => {
             // Define this since typings will be wrong if favorite is passed directly to the <Track>
             let track: Track.track = {
               title: fav.title,
               id: fav.id,
               artwork_url:
                 // Since sometimes null is a string in the artwork_url, LOL
                 switch (fav.artwork_url) {
                 | Some(url) => String.length(url) > 10 ? url : ""
                 | None => ""
                 },
               stream_url:
                 switch (fav.stream_url) {
                 | Some(url) => url
                 | None => ""
                 },
             };

             <Track
               key={string_of_int(index) ++ string_of_int(fav.id)}
               track
               currentTrack={state.currentTrack}
               onTogglePlay={_ => dispatch(TogglePlay)}
               onPlay={track => dispatch(Play(track))}
             />;
           },
         )
         |> Array.of_list
         |> Js.Array.slice(~start=0, ~end_=300) // Just cap the list at this number for now
       )}
    </div>
  </>;
};